#include <mp3Test/NOKIA5110.hpp>

constexpr uint8_t NOKIA5110::ascii[][5];

NOKIA5110::NOKIA5110(){
    m_sce = NULL;
    m_dc = NULL;
    m_reset = NULL;
    m_backlight = NULL;
}

NOKIA5110::NOKIA5110(GPIO *sce, GPIO *dc, GPIO *reset, PWM *pwm_pin){
    m_sce = sce;
    m_dc = dc;
    m_reset = reset;
    m_backlight = pwm_pin;
}

void NOKIA5110::select_display(){
    m_sce->setLow();
}

void NOKIA5110::deselect_display(){
    m_sce->setHigh();
}

void NOKIA5110::select_data_mode(){
    m_dc->setHigh();
}

void NOKIA5110::select_command_mode(){
    m_dc->setLow();
}

void NOKIA5110::send_command(uint8_t *command){
    select_command_mode();
    select_display();
    ssp1_exchange_byte(*command);
    deselect_display();
}

void NOKIA5110::send_commands(uint8_t *command, uint8_t size){
    for(int i = 0; i < size; i++){
        send_command(command);
    }
}

void NOKIA5110::send_data(uint8_t *data){
    select_data_mode();
    select_display();
    ssp1_exchange_byte(*data);
    deselect_display();
}


void NOKIA5110::send_data_bytes(uint8_t *data, uint8_t size){
    for(int i = 0; i < size; i++){
        send_data(data);
    }
}

void NOKIA5110::no_op(){
    send_command(&m_no_op_cmd);
}

void NOKIA5110::set_function(uint8_t function){
    send_command(&function);
}

void NOKIA5110::control_display(uint8_t configuration){
    set_function(m_basic_instr_cmd);
    send_command(&configuration);
}

void NOKIA5110::set_address_x(uint8_t x_address){
    set_function(m_basic_instr_cmd);
    send_command(&x_address);
}

void NOKIA5110::set_address_y(uint8_t y_address){
    set_function(m_basic_instr_cmd);
    send_command(&y_address);
}

void NOKIA5110::set_temperature(uint8_t temp_coefficient){
    set_function(m_extended_instr_cmd);
    send_command(&temp_coefficient);
}

void NOKIA5110::set_bias(uint8_t bias){
    set_function(m_extended_instr_cmd);
    send_command(&bias);
}

void NOKIA5110::set_Vop(uint8_t Vop){
    set_function(m_extended_instr_cmd);
    send_command(&Vop);
}

void NOKIA5110::set_xy_location(uint8_t x, uint8_t y){
    set_address_x((0x80 | x));
    set_address_y((0x40 | y));
}

void NOKIA5110::set_pixel(uint8_t x, uint8_t y, bool black_character){
    if( ((x >= 0) && (x <= LCD_WIDTH)) && ((y >= 0) && (y <= LCD_HEIGHT)) ){
        uint8_t vertical_shift = y % 8;
        if(black_character){
            display_map[x + ((y/8)*LCD_WIDTH)] |= (1 << vertical_shift);
        }else{
            display_map[x + ((y/8)*LCD_WIDTH)] &= ~(1 << vertical_shift);
        }
    }
}
/*
                         __        ______   _______          ______   _______  ______
                        |  \      /      \ |       \        /      \ |       \|      \
                        | $$     |  $$$$$$\| $$$$$$$\      |  $$$$$$\| $$$$$$$\\$$$$$$
                        | $$     | $$   \$$| $$  | $$      | $$__| $$| $$__/ $$ | $$
                        | $$     | $$      | $$  | $$      | $$    $$| $$    $$ | $$
                        | $$     | $$   __ | $$  | $$      | $$$$$$$$| $$$$$$$  | $$
                        | $$_____| $$__/  \| $$__/ $$      | $$  | $$| $$      _| $$_
                        | $$     \\$$    $$| $$    $$      | $$  | $$| $$     |   $$ \
                         \$$$$$$$$ \$$$$$$  \$$$$$$$        \$$   \$$ \$$      \$$$$$$



*/


void NOKIA5110::init_peripherals(){
    m_dc->setAsOutput();
    m_reset->setAsOutput();
    m_sce->setAsOutput();

    ssp1_init();

}

void NOKIA5110::reset(){
    select_display();
    m_reset->setLow();
    m_reset->setHigh();
    deselect_display();
}

void NOKIA5110::set_contrast(uint8_t contrast){
    set_Vop(contrast);
}

void NOKIA5110::update_display(){
    set_xy_location(0, 0);
    for(int16_t i = 0; i < ((LCD_WIDTH * LCD_HEIGHT) / 8); i++){
        send_data(&display_map[i]);
    }
}

void NOKIA5110::clear_display(bool white_background){
    if(white_background){
        for(int16_t i = 0; i < ((LCD_WIDTH * LCD_HEIGHT) / 8); i++){
            display_map[i] = 0x00;
        }
    }else{
        for(int16_t i = 0; i < ((LCD_WIDTH * LCD_HEIGHT) / 8); i++){
            display_map[i] = 0xFF;
        }
    }
    update_display();
}

bool NOKIA5110::init_display(){
    bool initialized = false;

    uint8_t Vop = 0xB0;
    uint8_t temperature_coefficient = 0x04;
    uint8_t bias = 0x14;

    uint8_t display_mode = 0x0C;

    init_peripherals();
    reset();
    set_Vop(Vop);
    set_temperature(temperature_coefficient);
    set_bias(bias);
    control_display(display_mode);
    clear_display(WHITE);

    initialized = true;
    return initialized;
}

void NOKIA5110::print_char(uint8_t x, uint8_t y, char character, bool black_character){
    uint8_t column = m_no_op_cmd;

    for(uint8_t col = 0; col < 5; col++){
        column = ascii[character - 0x20][col];

        for(uint8_t row = 0; row < 8; row++){
            if(column & (1 << row)){
                set_pixel((x+col), (y+row), black_character);
            }else{
                set_pixel((x+col), (y+row), !black_character);
            }
        }
    }
    update_display();
}

void NOKIA5110::print_string(uint8_t x, uint8_t y, const char *string, bool black_character){

    char character = ' ';
    int i = 0;
    while(*(string + i) != '\0'){
        character = *(string + i);

        print_char(x, y, character, black_character);
        x = x + 6;

        if(x > (LCD_WIDTH - 5)){
            x = 0;
            y = y + 8;
        }

        i++;
    }
}
