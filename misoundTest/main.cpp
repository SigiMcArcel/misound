#include <cstdio>
#include <mi/misound/Audio.h>
#include <mi/misound/Wave.h>

int main()
{
    printf("Hallo aus %s!\n", "misoundlibTest");
    misound::Audio* _audio = new misound::Audio();
   if (!_audio->addWave("/home/root/sounds/P1.wav","test",true))
    {
        return 1;
    }

   _audio->playWave("test", false, true);
   while (true)
   {
       _audio->setVolume(100);
       ::usleep(100000);
   }

    return 0;
}