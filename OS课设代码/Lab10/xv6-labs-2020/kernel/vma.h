struct vma {
    char* addr;
    uint64 len;
    struct file *file;
    char prot;
    char flag;
};