#include <windows.h>
#include <stdio.h>

void Press(int k)
{
    int scancode = MapVirtualKey(k, MAPVK_VK_TO_VSC);
    keybd_event(k, scancode, KEYEVENTF_EXTENDEDKEY, 0);
}

int main()
{
    HWND w = FindWindowA(NULL, "=-Wallgame-=");

    if(!w) return 0;
    SetForegroundWindow(w);
    Sleep(200);

    int D[] = { VK_UP, VK_LEFT, VK_DOWN, VK_RIGHT }, d = 0;
    char DD[] = "ULDR";
    int c = 1, i, m = 0, t = 0;

    while(1) {
        for(t = 0; t < 2; t++) {
            for(i = 0; i < c; i ++) {
                printf("%c ", DD[d]);
                Press(D[d % 4]);
                Sleep(60);
                m++;
            }
            if(m >= 624) return 0;
            d = (d + 1) % 4;
        }
        c++;
    }
}
