Semaphoore *offer1 = new Semaphore("", 1);
Semaphoore *offer2 = new Semaphore("", 1);
Semaphoore *offer3 = new Semaphore("", 1);
Semaphoore *finish = new Semaphore("", 1);

int i = 0;

void provider() {
    while(1) {
        if (i == 0) {
            // 放置组合1
            offer1->V();
        } else if (i == 1) {
            // 放置组合2
            offer2->V();
        } else if (i == 2) {
            // 放置组合3
            offer3->V();
        }
        i = (i + 1) % 3;
    }
    finish->P();
}

void smoker1() {
    while(1) {
        offer1->P();
        // 抽烟
        finish->V();
    }
}

void smoker2() {
    while(1) {
        offer2->P();
        // 抽烟
        finish->V();
    }
}

void smoker3() {
    while(1) {
        offer3->P();
        // 抽烟 
        finish->V();
    }
}