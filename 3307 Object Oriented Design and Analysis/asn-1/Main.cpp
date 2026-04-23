

#include "Main.h"
#include "FileManagerFrame.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    Manager_Frame *simple = new Manager_Frame();
    simple->Show(true);

    return true;
}
