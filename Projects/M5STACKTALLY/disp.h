#define POPUP_WIDTH     200
#define POPUP_HEIGHT     150

void SetOutlineRectangle(int x, int y, int width, int height, int borderwidth, int bordercolor, int paintcolor);

void drawHeader(bool init);
void drawFooter(bool init);
void disp_home(bool init);
void drawMain(bool init);
void exe_display(bool init);

void disp_popup(char* str);
