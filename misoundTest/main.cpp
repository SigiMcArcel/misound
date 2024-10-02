#include <cstdio>
#include <mi/misound/Audio.h>
#include <mi/misound/Wave.h>
#include <mi/miutils/Timer.h>

class test : public miutils::EventListener
{
    misound::AudioInterface& audio;
    miutils::Timer timer;
    int count;
public:
   

    test(misound::AudioInterface& a)
        :audio(a)
        , timer("29", this)
        ,count(0)
    {
        audio.addWave("s1p1", true);
        audio.addWave("s2p2", true);
        audio.addWave("s4p4", true);
        audio.addWave("s5p5", true);
        audio.addWave("s6p6", true);
        audio.addWave("s7p7", true);
        audio.addWave("s8p8", true);
        audio.addWave("s9p9", true);
        audio.addWave("s10p10", true);
        audio.addWave("s14p14", true);
        timer.Start(20);
    }

    virtual void eventOccured(void* sender, const std::string& name)
    {
        
        if (audio.isPlaying("s1p1"))
        {
            
            count++;
        }
        if (audio.isPlaying("s2p2"))
        {

            count++;
        }
    }

    void play1()
    {
        printf("play1\n");
        audio.playWave(std::string("s1p1"), false, true);
        audio.playWave("s2p2", false, true);
        audio.playWave("s4p4", false, true);
        audio.playWave("s5p5", false, true);
        audio.playWave("s6p6", false, true);
    }

    void play2()
    {
        printf("play2\n");
        audio.playWave("s1p1", false, true);
        audio.playWave("s2p2", false, true);
        audio.playWave("s4p4", false, true);
        audio.playWave("s5p5", false, true);
        audio.playWave("s6p6", false, true);
    }

    void stop1()
    {
        printf("stop1\n");
        audio.stopWave("s7p7");
        audio.stopWave("s2p2");
        audio.stopWave("s4p4");
        audio.stopWave("s5p5");
        audio.stopWave("s6p6");
    }
    void stop2()
    {
        printf("stop2\n");
        audio.stopWave("s7p7");
        audio.stopWave("s8p8");
        audio.stopWave("s9p9");
        audio.stopWave("s10p3");
        audio.stopWave("s14p14");
    }
};

int main()
{
    misound::Audio _Audio("plug:dmix1","/home/root/sounds");
    _Audio.setVolume(20);
    printf("Hallo aus %s!\n", "misoundlibTest");
    
    
    test test1(_Audio);
    test test2(_Audio);

   while (true)
   {
#if 0
       _Audio.setVolume(10);
       test1.play1();
       test2.stop2();
       ::sleep(10);
       test1.stop1();
       test2.play2();
       _Audio.setVolume(20);
#endif  
       ::sleep(10);
   }

    return 0;
}