#ifndef BUTTONS_H_f
#define BUTTONS_H_f

class buttons
{
    public:
    buttons();
    void init();
    void create_q();
    void next_isr();
    void prev_isr();
    void pause_isr();

    void send_mp3_cmd(mp3Command toSend);

    bool mp3PlayerNextHandler(str& cmdParams, CharDev& output, void* pDataParam);

    bool mp3PlayerPrevHandler(str& cmdParams, CharDev& output, void* pDataParam);

    bool mp3PlayerPauseHandler(str& cmdParams, CharDev& output, void* pDataParam);

    bool mp3PlayerResumeHandler(str& cmdParams, CharDev& output, void* pDataParam);
    ~buttons();

};
#endif
