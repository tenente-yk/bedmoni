#ifndef __MN_H
#define __MN_H

/*
MEN file structure:
Consists of merged packets, ends by 4 MI_EOFs
Converts to *.men from *.mn resource files by using a "mn" utility
packet structure:
offset   length   description
0        1        MI_BEG
1        2        menu item packet len
3        1        menu item type (enum MI_)
4        2        menu item string id
6        1        menu item inpfoc id
7        1        menu item infoc item
8        2        x
10       2        y
12       2        cx
14       2        cy
16       1        reserved
17       1        reserved
18       1        MI_END
(I.e.: 0x05 ... 0x00 0x00 0x06 0x07 0x07 0x07 0x07)
*/

#define MI_BEG        0x05
#define MI_END        0x06
#define MI_EOF        0x07

enum
{
  MI_NONE = 0,       // 0
  MI_STATIC,         // 1
  MI_INPFOC,         // 2
  MI_SPINBUTTON,     // 3
  MI_CHECKBOX,       // 4
  MI_SLIDER,         // 5
  MI_PROGRESSBAR,    // 6
  MI_LISTBUTTON,     // 7
  MI_RADIOBUTTON     // 8
};

#endif // __MN_H
