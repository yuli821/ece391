// You may add up to 3 elements to this struct.
// The type of synchronization primitive you may use is SpinLock.

/*
    Restriction on the biscuit number: Though there's no limits to the number of
    biscuits, the 
*/
typedef struct biscuit_juice_lock{
    volatile int juice_num;
    volatile int biscuit_num;
    spinlock_t* lock;
}fd_lock_t;


void produce_biscuit(fd_lock_t* fd) {
    spin_lock(fd->lock);
    fd->biscuit_num++;
    spin_unlock(fd->lock);
    return;
}

void consume_biscuit(fd_lock_t* fd) {
    while(1){
        spin_lock(fd->lock);
        if(fd->juice_num == 0 && fd->biscuit_num != 0){
            fd->biscuit_num--;
            spin_unlock(fd->lock);
            return;
        }
        spin_unlock(fd->lock);
    }
}

int produce_juice(fd_lock_t* fd) {
    spin_lock(fd->lock);
    if(fd->juice_num == 4){
        spin_unlock(fd->lock);
        return -1;
    }
    fd->juice_num++;
    spin_unlock(fd->lock);
    return 0;
}

void consume_juice(fd_lock_t* fd) {
    while(1){
        spin_lock(fd->lock);
        if (fd->juice_num != 0){
            fd->juice_num--;
            spin_unclock(fd->lock);
            return;
        }
        spin_unclock(fd->lock);
    }
}

