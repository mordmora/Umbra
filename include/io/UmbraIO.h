#pragma once

#include<string>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<stdexcept>

namespace umbra {

    class UmbraIOExceptionManager {



    };

    class UmbraIOMemMapMODULE : UmbraIOExceptionManager {
        public:
        UmbraIOMemMapMODULE(std::string& FileIO) : FileIO(FileIO) {}

        void* getFContent() {

            fd = fopen(FileIO.c_str(), 0O_RDONLY);
            if (fd == -1){

            }




        }

        ~UmbraIOMemMapMODULE(){
            if(MemMapADDR != (void*)-1){
                mmunmap(MemMapADDR);
            }
        }


        UmbraIOMemMapMODULE (const UmbraIOMemMapMODULE) = delete;
        UmbraIOMemMapMODULE operator=(const UmbraIOMemMapMODULE) = delete;

        private:
        std::string& FileIO;
        size_t fsize = 0;
        void* MemMapADDR = nullptr;

        #ifdef __UNIX__

        #else

        #endif

    }
};
