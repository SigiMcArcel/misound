#include <cstdio>
#include <cAudio.h>
#include <cWave.h>

int main()
{
    printf("Hallo aus %s!\n", "misoundlibTest");
    cAudio* _audio = new cAudio();
   if (!_audio->addWave("/home/root/test.wav","test",true))
    {
        return 1;
    }

   _audio->playWave("test", false, true);
   while (true)
   {
       ::usleep(100000);
   }

    return 0;
}