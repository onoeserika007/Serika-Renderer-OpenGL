#pragma once

namespace Serika {
    template<typename T>
    class UUID {
    public:
        UUID() : uuid_(++uuidCounter_) {}

        inline int get() const {
            return uuid_;
        }

    private:
        int uuid_ = 0; // 0代表不存在，和opengl类似
        static int uuidCounter_;
    };

    template<typename T>
    int UUID<T>::uuidCounter_ = 0;

}

