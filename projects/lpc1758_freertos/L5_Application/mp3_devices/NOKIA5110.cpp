

LCDDisplayNokia5110::LCDDisplayNokia5110() : m_no_op_cmd(0x00), m_basic_instr_cmd(0x20), m_extended_instr_cmd(0x21){
    // user didn't specify pins in the constructor
    m_sce = nullptr;
    m_dc = nullptr;
    m_reset = nullptr;
    m_backlight = nullptr;
}


LCDDisplayNokia5110::LCDDisplayNokia5110(GPIO *sce_pin, GPIO *dc_pin, GPIO *res_pin, PWM *backlight_pin) : m_no_op_cmd(0x00), m_basic_instr_cmd(0x20), m_extended_instr_cmd(0x21){
    //Assign pointers to private pointer data members, so they can access user's pins
    m_sce = sce_pin; //address that the sce_pin pointer points to will be saved to m_sce
    m_dc = dc_pin;
    m_reset = res_pin;
    m_backlight = backlight_pin;
}

LCDDisplayNokia5110::~LCDDisplayNokia5110(){}

void LCDDisplayNokia5110::selectDisplay(){
    m_sce->setLow();
}

void LCDDisplayNokia5110::deselectDisplay(){
    m_sce->setHigh();
}

void LCDDisplayNokia5110::selectDataMode(){
    m_dc->setHigh();
}

void LCDDisplayNokia5110::selectCommandMode(){
    m_dc->setLow();
}

void LCDDisplayNokia5110::sendCommand(uint8_t *cmd){
    selectCommandMode();
    selectDisplay();
    ssp1_exchange_byte(*cmd); // send command byte over DIN pin
    deselectDisplay();
}

void LCDDisplayNokia5110::sendCommands(uint8_t *cmd, uint8_t size){
    for(uint8_t i = 0; i < size; i++){
        sendCommand(cmd);
    }
}

void LCDDisplayNokia5110::sendData(uint8_t *data){
    selectDataMode();
    selectDisplay();
    ssp1_exchange_byte(*data); //send data byte over DIN pin
    deselectDisplay();
}

void LCDDisplayNokia5110::sendDataBytes(uint8_t *data, uint8_t size){
    for(uint8_t i = 0; i < size; i++){
        sendData(data);
    }
}

void LCDDisplayNokia5110::nop(){
    sendCommand(&m_no_op_cmd);
}

void LCDDisplayNokia5110::setFunction(uint8_t specify_function){
    sendCommand(&specify_function);
}

void LCDDisplayNokia5110::controlDisplay(uint8_t configure_display){
    setFunction(m_basic_instr_cmd); 
    sendCommand(&configure_display);
}

/**
 * Sets Y vector address of DDRAM, must fall between banks
 * , which are rows 8-pixels HIGH each, 0 <= Y <= 5
 */
void LCDDisplayNokia5110::setYAddressDDRAM(uint8_t y_address){
    setFunction(m_basic_instr_cmd); //set function to use basic instruction set
    sendCommand(&y_address);
}

void LCDDisplayNokia5110::setXAddressDDRAM(){
    setFunction(m_basic_instr_cmd);
    sendCommand(&x_address);
}

void LCDDisplayNokia5110::controlTemperature(uint8_t set_temp_coeff){
    setFunction(m_extended_instr_cmd);
    sendCommand(&set_temp_coeff);
}

/**
 * Sets Bias System (BSx) multiplex ratio rate. The rate indicates how many
 * voltage reference points are created to drive the LCD.
 */
void LCDDisplayNokia5110::setBiasSystem(uint8_t bias_number){
    setFunction(m_extended_instr_cmd); //set function to use extended instruction set
    sendCommand(&bias_number);
}

  /*  Equation that Vop is applied in: V_LCD = a + (Vop6 to Vop0) * b[V]
 */
void LCDDisplayNokia5110::setVop(uint8_t vop_value){
    setFunction(m_extended_instr_cmd);

    sendCommand(&vop_value);
}

void LCDDisplayNokia5110::jumpToXY(uint8_t x, uint8_t y){
    setXAddressDDRAM( (0x80 | x) ); 

    setYAddressDDRAM( (0x40 | y) ); 
}

void LCDDisplayNokia5110::setPixel(uint8_t x, uint8_t y, bool black_white){
    if( (x >= 0) && (x < LCD_WIDTH) && (y >= 0) && (y < LCD_HEIGHT) ){
        uint8_t shift = y % 8;
        if(black_white){
            displayMap[x + (y/8)*LCD_WIDTH] |= 1 << shift;
        }
        else{
            displayMap[x + (y/8)*LCD_WIDTH] &= ~(1 << shift);
        }
    }
}

/*

UI

*/

void LCDDisplayNokia5110::initPinPeripherals(){
    m_dc->setAsOutput(); 
    m_reset->setAsOutput(); 
    m_sce->setAsOutput(); 
    ssp1_init();
}

void LCDDisplayNokia5110::activateLEDBackLight(){
    m_backlight->set(75); //PMW 75%
}

void LCDDisplayNokia5110::deactivateLEDBackLight(){
    m_backlight->set(0); //set PWM Duty Cycle to 0% HIGH, 100% LOW
}

void LCDDisplayNokia5110::hardReset(){
    selectDisplay();
    m_reset->setLow();
    m_reset->setHigh();

    deselectDisplay();
}

void LCDDisplayNokia5110::setContrast(uint8_t contrast){
    setVop( (0x80 | contrast) );
}

/**
 * Transfer changes made to the 504 byte displayMap array to the display's DDRAM
 */
void LCDDisplayNokia5110::updateDisplay(){
    jumpToXY(0, 0);
    for(uint16_t i = 0; i < (LCD_WIDTH * LCD_HEIGHT / 8); i++){
        sendData(&displayMap[i]);
    }
}

void LCDDisplayNokia5110::clearDisplay(bool black_white){
    for(uint16_t i = 0; i < (LCD_WIDTH * LCD_HEIGHT /8); i++){
        if(black_white){
            displayMap[i] = 0xFF; //insert black pixel
        }
        else{
            displayMap[i] = 0x00; //insert white pixel
        }
    }
    updateDisplay();
}



bool LCDDisplayNokia5110::initDisplay(){
    bool init_display = false;
    //LCD operational voltage (VLCD), Vop6 to Vop0 = 48 in decimal
    uint8_t vop = 0xB0; //= 0b10110000

    //sets temperature coefficient
    uint8_t temp_coefficient = 0x04; //= 0b00000100

    //LCD bias mux rate is 1:40/1:34, n = 3,
    uint8_t bias_number = 0x14; //= 0b00010100

    //Configures display to be in normal mode (D = 1 and E = 0)
    uint8_t display_mode = 0x0C; //= 0b00001100


    initPinPeripherals(); //Setup GPIO and SSP1 for LCD Display
    hardReset(); //All internal registers are reset at pad 31
    activateLEDBackLight(); //Turn on LED backlight dimming to HIGH
    setVop(vop); //sets VLCD = 3.00 + 48 * 0.06 = 5.88
    controlTemperature(temp_coefficient); //set VLCD temperature coefficient 0
    setBiasSystem(bias_number); //sets bias number "n" = 3
    controlDisplay(display_mode); //sets display in normal mode
    clearDisplay(WHITE); //clears LCD display pixels

    init_display = true;
    return init_display;
}

void LCDDisplayNokia5110::printCharacter(uint8_t x, uint8_t y, char character, bool black_white){
    uint8_t column = m_no_op_cmd;
    //Draw the ascii character passed in by user to fill 8x5 area of DDRAM
    // 5 columns (x) per character
    for(uint8_t col = 0; col < 5; col++) {
        // Since 0x00 to 0x19 are non-displayable characters, subtract the
        // first 0x20 rows to store characters that are viewable
        column = ascii[character - 0x20][col];

        // 8 rows (y) per character
        for(uint8_t row = 0; row < 8; row++) {
            //test if each row[0-7] in column[0] are set, then move onto next row column
            if(column & (1 << row)){
                //set pixel in X and Y coordinate and draw char portion until
                //the whole ascii char has been drawn
                setPixel((x + col), (y + row), black_white);
            }
            else //if bit isn't set, then clear pixel
            {
                setPixel((x + col), (y + row), !black_white);
            }
        }
    }
    //Send updated displayMap to DDRAM to print char(s) onto display
    updateDisplay();
}

void LCDDisplayNokia5110::printString(uint8_t x, uint8_t y, const char *c_string, bool black_white){
    char letter = ' ';
    for(uint8_t i = 0; *(c_string + i) != '\0'; i++){
        letter = *(c_string + i);

        printCharacter(x, y, letter, black_white);

        //move along X-axis 5 columns to the right to create
        //a space between the prior char and the next char
        //The reason is because each char takes up 5 columns
        x = x + 5;

        //clear the row (8 spaces along Y-axis) column (X-axis) between the prior char and the next char
        for(uint8_t row = y; row < y + 8; row++){
            setPixel(x, row, !black_white);
        }

        //move along X-axis 1 column to the right, so we can print the next char
        x = x + 1;

        //check if theres not enough room for 5 column char on current row
        //if true "wrap around" to next row starting at column 0
        if(x > (LCD_WIDTH - 5)){
            x = 0; //start at column 0
            y = y + 8; //start at next row
        }
    }
}