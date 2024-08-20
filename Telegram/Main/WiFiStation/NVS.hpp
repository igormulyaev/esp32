#ifndef NVS_HPP
#define NVS_HPP

class NVS
{
    public:
        NVS() {};
        ~NVS() {};

        void Init();
    
    private:
        static const char * const TAG;
};

#endif