#pragma once

namespace KCore {
    class BaseTask {
    public:
        void invoke() {
            onTaskBeforeStart();
            performTask();
            onTaskComplete();
        }

        virtual void onTaskBeforeStart() {};

        virtual void performTask() = 0;

        virtual void onTaskComplete() {};
    };
}