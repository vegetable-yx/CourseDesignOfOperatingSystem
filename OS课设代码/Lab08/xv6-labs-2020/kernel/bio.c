// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"
#define HASH_NUM 13

struct {
  struct spinlock lock;
  struct buf buf[NBUF];
} bcache;

struct bmem {
  struct spinlock lock;
  struct buf head;
};

static struct bmem hash_table[HASH_NUM];

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  b = bcache.buf;
  for (int i = 0; i < HASH_NUM; i++) {
    initlock(&(hash_table[i].lock), "bcache.bucket");
    int count = NBUF / HASH_NUM;
    if (i < NBUF % HASH_NUM) count++;

    for (int j = 0; j < count; j++, b++) {
      b->blockno = i;
      b->next = hash_table[i].head.next;
      hash_table[i].head.next = b;
    }
  }

  for (b = bcache.buf; b < bcache.buf + NBUF; b++)
    initsleeplock(&b->lock, "buffer");
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int num = blockno % HASH_NUM;
  acquire(&(hash_table[num].lock));

  for (b = hash_table[num].head.next; b != 0; b = b->next) {
    if (b->dev == dev && b->blockno == blockno) {
      b->refcnt++;
      release(&(hash_table[num].lock));
      acquiresleep(&b->lock);
      return b;
    }
  }

  int minist = __INT_MAX__;
  struct buf* replace = 0;

  for (b = hash_table[num].head.next; b != 0; b = b->next) {
    if (b->refcnt == 0 && b->tick < minist) {
      replace = b;
      minist = b->tick;
    }
  }
  if (replace) {
    replace->dev = dev;
    replace->blockno = blockno;
    replace->valid = 0;
    replace->refcnt = 1;
    release(&(hash_table[num].lock));
    acquiresleep(&replace->lock);
    return replace;
  }

  acquire(&bcache.lock);

  int tag, rnum;
  do {
    tag = 1;
    for (b = bcache.buf; b < bcache.buf + NBUF; b++) {
      if (b->refcnt == 0 && b->tick < minist) {
        replace = b;
        minist = b->tick;
      }
    }
    if (replace) {
      rnum = replace->blockno % HASH_NUM;
      acquire(&(hash_table[rnum].lock));
      if (replace->refcnt != 0) {       // To avoid locked by others between find and acquire.
        release(&(hash_table[rnum].lock));
        tag = 0;
      }
    } else {
      panic("bget: no buffers");
    }
  } while (!tag);
  struct buf *pre = &hash_table[rnum].head;
  struct buf *post = hash_table[rnum].head.next;
  while (post != replace) {
    pre = pre->next;
    post = post->next;
  }
  pre->next = post->next;
  release(&(hash_table[rnum].lock));
  replace->next = hash_table[num].head.next;
  hash_table[num].head.next = replace;
  release(&bcache.lock);
  replace->dev = dev;
  replace->blockno = blockno;
  replace->valid = 0;
  replace->refcnt = 1;
  release(&(hash_table[num].lock));
  acquiresleep(&replace->lock);
  return replace;
}


// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  uint64 num = b->blockno % HASH_NUM;
  acquire(&(hash_table[num].lock));
  b->refcnt--;
  if (b->refcnt == 0) b->tick = ticks;
  release(&(hash_table[num].lock));
}

void
bpin(struct buf *b) {
  uint64 num = b->blockno % HASH_NUM;
  acquire(&(hash_table[num].lock));
  b->refcnt++;
  release(&(hash_table[num].lock));
}

void
bunpin(struct buf *b) {
  uint64 num = b->blockno % HASH_NUM;
  acquire(&(hash_table[num].lock));
  b->refcnt--;
  release(&(hash_table[num].lock));
}


