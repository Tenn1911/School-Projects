/*
*Author: Tennyson Hung
*Description: This file contains the implementations of the functions declared in FileManagerFrame.h.
*Date: 2026-02-03
 * */


#include "FileManagerFrame.h"

namespace fs = std::filesystem;

/*
 *Name: Manager_Frame 
 *Description: This is the constructor for the Manager_Frame class. Which calls other functions, and initializes the application.
 *Parameters: None
 *Return: None
 * */
Manager_Frame::Manager_Frame() 
	: wxFrame(NULL, -1, "File Manager", wxDefaultPosition, wxSize(800, 550))
{
   try{
    
    CreateMenu();
    InitializeStatusBar();
  
    CreateLayout(this);
    Centre();
    
    } catch (const std::exception& e) {
        wxLogError("Error in Manager_Frame Constructor: %s", e.what());
    }
}

/*
 *Name: CreateLayout
 *Description: This function creates the layout for the address bar and the file list
 *Parameters: wxWindow* parent, this is the parent window to attach the layout to
 *Return: None
 * */
void Manager_Frame::CreateLayout(wxWindow* parent){

    try{

    wxBoxSizer *vBox = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *directoryBox = new wxBoxSizer(wxHORIZONTAL);


    InitializeAddressBar(parent);
  

    directoryBox->Add(GetAddressBar(), 1);
    vBox->Add(directoryBox, 0, wxEXPAND | wxBOTTOM |wxLEFT | wxRIGHT | wxTOP, 10);

    InitializeFileListView(parent);
    vBox->Add(GetFileListView(), 1, wxEXPAND);

    parent->SetSizer(vBox);

    } catch (const std::exception& e) {
        wxLogError("Error in CreateLayout: %s", e.what());
    }
}

/*
 *Name: InitializeAddressBar
 *Description: This function initializes that address bar with the current directory
 *Parameters: wxWindow* parent, this is the parent window to attach the address bar to
 *Return: None
 * */
void Manager_Frame::InitializeAddressBar(wxWindow* parent){

    try{

    addressBar = new wxTextCtrl(parent, ADDRESSBAR_ID, myFS.GetCurrentDirectory().string(), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    //connect the event handler
    Connect(ADDRESSBAR_ID, wxEVT_TEXT_ENTER,
      wxCommandEventHandler(Manager_Frame::OnAddressBarEnter));    
    
    } catch (const std::exception& e) {
        wxLogError("Error in InitializeAddressBar: %s", e.what());
    }
}


/*
 *Name: InitializeFileListView
 *Description: This file initializes ListCtrl of the the file list for the current directory
 *Parameters: wxWindow* parent, this is the parent window to attach the address bar to
 *Return: None
 * */
void Manager_Frame::InitializeFileListView(wxWindow* parent){

    try{
    
    fileListView = new wxListCtrl(parent, FILELISTVIEW_ID, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);

    fileListView->InsertColumn(0, "Name", wxLIST_FORMAT_LEFT, 200);
    fileListView->InsertColumn(1, "Type", wxLIST_FORMAT_LEFT, 100);
    fileListView->InsertColumn(2, "Size", wxLIST_FORMAT_LEFT, 100);
    fileListView->InsertColumn(3, "Date", wxLIST_FORMAT_LEFT, 300);

    //display the file list
    RefreshFileList();

    //connect the event handler
    Connect(FILELISTVIEW_ID, wxEVT_LIST_ITEM_ACTIVATED,
      wxListEventHandler(Manager_Frame::OnDoubleClick));

    return;

    } catch (const std::exception& e) {
        wxLogError("Error in InitializeFileListView: %s", e.what());
    }
}

/*
 *Name: RefreshFileList
 *Description: This function resets the current directory's file list being displayed
 *Parameters: None
 *Return: None
 * */
void Manager_Frame::RefreshFileList() {
    try{

    fileListView->DeleteAllItems();
    std::vector<FileInfo> files = myFS.GetCurrentDirectoryFiles();

    //the first file is always the '..' directory
    long row = fileListView->InsertItem(0,wxString(".."));
    fileListView->SetItem(row, 1, wxString("Dir"));
    fileListView->SetItem(row, 2, wxString::Format("0"));
    fileListView->SetItem(row, 3, wxString(myFS.getFileTime(myFS.GetCurrentDirectory())));
    
    
    for (size_t i = 0; i < files.size(); ++i) {
        //Insert the row and set Name column
        long index = fileListView->InsertItem(i + 1, wxString(files[i].name)); 

        //Set Type, Size, and date coulmns
        fileListView->SetItem(index, 1, wxString(files[i].type));
        fileListView->SetItem(index, 2, wxString::Format("%lu KB", files[i].size / 1024));
        fileListView->SetItem(index, 3, wxString(files[i].dateModified));

    }

    } catch (const std::exception& e) {
        wxLogError("Error in RefreshFileList: %s", e.what());
    }
}

/*
 *Name: GetSelectedFilePath
 *Description: This function gets the selected file, in the wxListCtrl
 *Parameters: None
 *Return: Returns the file that is selected, or an empty path if none are selected
 * */
fs::path Manager_Frame::GetSelectedFilePath(){
    try{
    //find seleceted item
    long itemIndex = fileListView->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    //If nothing is selected, return an empty path
    if (itemIndex == -1) {
        return fs::path(); 
    }

    //Get the filename from the first column
    wxString name = fileListView->GetItemText(itemIndex, 0);

    //Combine with the current directory to return the full path
    return myFS.GetCurrentDirectory() / name.ToStdString();
    
    } catch (const std::exception& e) {
        wxLogError("Error in SetStatus: %s", e.what());
	return fs::path();
    }
}

/*
 *Name: InitializeStatusBar
 *Description: This function initializes the status bar
 *Parameters: None
 *Return: None
 * */
void Manager_Frame::InitializeStatusBar(){
    try{ 
    statusBar = CreateStatusBar(1);
    SetStatus("Welcome to the wxWidgets File Manager");

    } catch (const std::exception& e) {
        wxLogError("Error in SetStatus: %s", e.what());
    }
}

/*
 *Name: SetSatus
 *Description: This function sets the status of the status bar to the given string 
 *Parameters: std::string status, is the status to be displayed
 *Return: None
 * */
void Manager_Frame::SetStatus(std::string status){
    try{
    statusBar->SetStatusText(status);

    } catch (const std::exception& e) {
        wxLogError("Error in SetStatus: %s", e.what());
    }
}
/*
 *Name: CreateMenu
 *Description: This function creates the options menu for the operations on the files
 *Parameters: None
 *Return: None
 * */
void Manager_Frame::CreateMenu(){
  
    try{

    wxMenuBar *menubar = new wxMenuBar;
    wxMenu *Options = new wxMenu;

    //Create and add all menu buttons
    Options->Append(OPEN_ID, wxT("Open\tCtrl+O"));
    Options->Append(NEW_ID, wxT("New\tCtrl+N"));
    Options->Append(RENAME_ID, wxT("Rename\tCtrl+E"));
    Options->Append(DELETE_ID, wxT("Delete\tCtrl+D"));
    Options->Append(COPY_ID, wxT("Copy\tCtrl+C"));
    Options->Append(CUT_ID, wxT("Cut\tCtrl+X"));
    Options->Append(PASTE_ID, wxT("Paste\tCtrl+V"));
    Options->Append(REFRESH_ID, wxT("Refresh\tCtrl+R"));
    Options->Append(wxID_EXIT, wxT("Exit\tCtrl+W"));

    Options->AppendSeparator();
    
    menubar->Append(Options, wxT("&Options"));
    SetMenuBar(menubar);

    //Connect all buttons to event functions
    Connect(OPEN_ID, wxEVT_COMMAND_MENU_SELECTED,
      wxCommandEventHandler(Manager_Frame::OnOpen));

    Connect(NEW_ID, wxEVT_COMMAND_MENU_SELECTED,
      wxCommandEventHandler(Manager_Frame::OnNew));

    Connect(RENAME_ID, wxEVT_COMMAND_MENU_SELECTED,
      wxCommandEventHandler(Manager_Frame::OnRename));

    Connect(DELETE_ID, wxEVT_COMMAND_MENU_SELECTED,
      wxCommandEventHandler(Manager_Frame::OnDelete));

    Connect(COPY_ID, wxEVT_COMMAND_MENU_SELECTED,
      wxCommandEventHandler(Manager_Frame::OnCopy));

    Connect(CUT_ID, wxEVT_COMMAND_MENU_SELECTED,
      wxCommandEventHandler(Manager_Frame::OnCut));

    Connect(PASTE_ID, wxEVT_COMMAND_MENU_SELECTED,
      wxCommandEventHandler(Manager_Frame::OnPaste));

    Connect(REFRESH_ID, wxEVT_COMMAND_MENU_SELECTED,
      wxCommandEventHandler(Manager_Frame::OnRefresh));

    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
      wxCommandEventHandler(Manager_Frame::OnQuit));
    Centre();	
    
    } catch (const std::exception& e) {
        wxLogError("Error in CreateMenu: %s", e.what());
    }
}

/*
 *Name: hasSelected
 *Description: This function checks if a file from the file list has been selected
 *Parameters: None
 *Return: Returns true if a file is selected, false if none are selected
 * */
bool Manager_Frame::hasSelected(){
    try {

    long selectedItem = fileListView->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (selectedItem == -1) {
        return false;
    }
    return true;

    } catch (const std::exception& e) {
        wxLogError("Error in hasSelected: %s", e.what());
	return false;
    }
}

/*
 *Name: OnAddressBarEnter
 *Description: This function will change the current directory when a valid directory is entered into the address bar
 *Parameters: wxCommandEvent& event, is the event that occured
 *Return: None
 * */
void Manager_Frame::OnAddressBarEnter(wxCommandEvent& event) {
    try{
    wxString rawPath = addressBar->GetValue();
    std::string pathString = rawPath.ToStdString();

    // Attempt to change directory
    if (myFS.SetCurrentDirectory(pathString)) {
        // Update the bar with the clean path
        addressBar->SetValue(myFS.GetCurrentDirectory().string());
	RefreshFileList();

    } 
    else {
        wxMessageBox("Directory not found or inaccessible.");
        addressBar->SetValue(myFS.GetCurrentDirectory().string());
    }
    } catch (const std::exception& e) {
        wxLogError("Error in OnAddressBarEnter: %s", e.what());
    }

}

/*
 *Name: OnDoubleClick
 *Description: This function Opens the file or directory that is selected from the file list
 *Parameters: wxListEvent& event, is the event that occured
 *Return: None
 * */
void Manager_Frame::OnDoubleClick(wxListEvent& event) { // Changed to wxListEvent
    try {

 	long itemIndex = event.GetIndex();

        wxString name = fileListView->GetItemText(itemIndex, 0);

        fs::path targetPath = myFS.GetCurrentDirectory() / name.ToStdString();

	
	if (!fs::exists(targetPath)) {
            wxLogError("Path does not exist: %s", targetPath.string());
            return;
        }

	SetSelectedFilePath(targetPath);
        OnOpen(event);

    } catch (const std::exception& e) {
        wxLogError("Error in OnDoubleClick: %s", e.what());
    }
}

/*
 *Name:OnOpen
 *Description: This function preforms the open operation on the file which is currently selected when the menu buttom is pressed, or the shortcut is inputted
 *Parameters: wxCommandEvent & event, is the event that occured
 *Return: None
 * */
void Manager_Frame::OnOpen(wxCommandEvent & events){
 
    try{
    if (!hasSelected()) {
        //Nothing is selected, exit early to prevent crash
	wxLogMessage("No file selected");
        return;
    }

    fs::path targetPath = fs::absolute(GetSelectedFilePath());

    if (!fs::exists(targetPath)) {
            wxLogError("File does not exist: %s", targetPath.string());
            return;
    }
    //If it's a folder, enter it. If it's a file, open it.
    if (fs::is_directory(targetPath)) {
        if (myFS.SetCurrentDirectory(targetPath.string())) {
            RefreshFileList();     
            UpdateAddressBar();     
        }
    } else {
    	if(fs::exists(targetPath)&&fs::is_regular_file(targetPath)){    
	    wxString wxPathString = wxString::FromUTF8(targetPath.string());
            if (!wxLaunchDefaultApplication(wxPathString)) {
                wxLogError("Failed to open. Error code: %ld", wxSysErrorCode());
            }
	}
	else{
	    wxLogMessage("File could not be opened.");
	}
    }
    } catch (const std::exception& e) {
        wxLogError("Error in OnNew: %s", e.what());
    }
}

/*
 *Name: OnNew
 *Description: This function preforms the create operation on the file which is currently selected when the menu buttom is pressed, or the shortcut is inputted
 *Parameters: wxCommandEvent & event, is the event that occured
 *Return: None
 * */
void Manager_Frame::OnNew(wxCommandEvent & event){

    try{

    wxTextEntryDialog dlg(this, "Enter directory name:","Input Required","",wxOK | wxCANCEL);

    if (dlg.ShowModal() == wxID_OK) {
        wxString name = dlg.GetValue();
    	if(!myFS.CreateNewDirectory(name.ToStdString())){
           wxLogMessage("Could not create Directory.");
    	}
        RefreshFileList();
    }

    } catch (const std::exception& e) {
        wxLogError("Error in OnNew: %s", e.what());
    }
}

/*                                                                                                                                                   *Name: OnRename
 *Description: This function preforms the rename operation on the file which is currently selected when the menu buttom is pressed, or the shortcut is inputted
 *Parameters: wxCommandEvent & event, is the event that occured
 *Return: None
 * */
void Manager_Frame::OnRename(wxCommandEvent & event){

    try{

    if (!hasSelected()) {
        // Nothing is selected, exit early to prevent crash
        wxLogMessage("No file selected");
        return;
    }

    fs::path targetPath = fs::absolute(GetSelectedFilePath());

    if(fs::exists(targetPath)&&fs::is_regular_file(targetPath)){
        wxTextEntryDialog dlg(this, "Enter new name:","Input Required","",wxOK | wxCANCEL);
    	if (dlg.ShowModal() == wxID_OK) {
            wxString name = dlg.GetValue();
            if(!myFS.RenameFile(targetPath, name.ToStdString())){
               wxLogMessage("Could not rename file.");
            }
            RefreshFileList();
        }   
     
    }
    else{
      wxLogMessage("Could not rename what you have seleceted");
    }

    } catch (const std::exception& e) {
        wxLogError("Error in OnRename: %s", e.what());
    }
}

/*
 *Name: OnDelete
 *Description: This function preforms the delete operation on the file which is currently selected when the menu buttom is pressed, or the shortcut is inputted
 *Parameters: wxCommandEvent & event, is the event that occured
 *Return: None
 * */
void Manager_Frame::OnDelete(wxCommandEvent & event){
	
   try{

    if (!hasSelected()) {
        // Nothing is selected, exit early to prevent crash
        wxLogMessage("No file selected");
        return;
    }

    fs::path targetPath = fs::absolute(GetSelectedFilePath());
    
   
    if(fs::exists(targetPath)&&fs::is_regular_file(targetPath)){
        wxMessageDialog dlg(this, "Are you sure you want to delete this","Alert",wxOK | wxCANCEL);
        if (dlg.ShowModal() == wxID_OK) {
	    if(!myFS.DeleteFile(targetPath)){
                wxLogMessage("Could not delete file.");
                return;
	    }
            RefreshFileList();
        }
    }
    else{
       wxLogMessage("Cannot delete what you have selected");
    }

    } catch (const std::exception& e) {
        wxLogError("Error in OnDelete: %s", e.what());
    }
}

/*
 *Name: OnCopy
 *Description: This function preforms the copy operation on the file which is currently selected when the menu buttom is pressed, or the shortcut is inputted
 *Parameters: wxCommandEvent & event, is the event that occured
 *Return: None
 * */
void Manager_Frame::OnCopy(wxCommandEvent & event){

    try{
    if (!hasSelected()) {
        // Nothing is selected, exit early to prevent crash
        wxLogMessage("No file selected");
        return;
    }

    fs::path targetPath = fs::absolute(GetSelectedFilePath());

    
    if(fs::exists(targetPath)&&fs::is_regular_file(targetPath)){
        myFS.SetClipboard(targetPath, false);
        SetStatus(targetPath.string() +" has been copied.");
    }
    else{
        wxLogMessage("Could not copy what you have seleceted");
    }
    } catch (const std::exception& e) {
        wxLogError("Error in OnCopy: %s", e.what());
    }
}

/*
 *Name: OnCut
 *Description: This function preforms the cut operation on the file which is currently selected when the menu buttom is pressed, or the shortcut is inputted
 *Parameters: wxCommandEvent & event, is the event that occured
 *Return: None
 * */
void Manager_Frame::OnCut(wxCommandEvent & event){

    try{
    if (!hasSelected()) {
        // Nothing is selected, exit early to prevent crash
        wxLogMessage("No file selected");
        return;
    }

    fs::path targetPath = fs::absolute(GetSelectedFilePath());


    if(fs::exists(targetPath)&&fs::is_regular_file(targetPath)){
	myFS.SetClipboard(targetPath, true);
        SetStatus(targetPath.string() +" has been cut.");
    }
    else{
    	wxLogMessage("Could not cut what you have seleceted");
    }
    } catch (const std::exception& e) {
        wxLogError("Error in OnCut: %s", e.what());
    } 
}

/*
 *Name: OnPaste 
 *Description: This function pastes the copied file, into the current directory when the menu buttom is pressed, or the shortcut is inputted
 *Parameters: wxCommandEvent & event, is the event that occured
 *Return: None
 * */
void Manager_Frame::OnPaste(wxCommandEvent & event){

  try{

    fs::path targetDir = myFS.GetCurrentDirectory();

    std::string fileToPaste = myFS.GetClipboardPath().filename().string();

    bool isoverwrite = false;

    if(fs::exists(targetDir)){

        std::vector<FileInfo> fileList = myFS.GetCurrentDirectoryFiles();
	for(size_t i = 0; i<fileList.size(); i++){
	   if(fileToPaste.compare(fileList[i].name) == 0){
	       wxMessageDialog dlg(this, "Do you want to overwrite the other file","Alert",wxOK | wxCANCEL);
               if (dlg.ShowModal() == wxID_OK) {
	           isoverwrite = true;
	       } 
	   }
	}

        if(!myFS.Paste(targetDir,isoverwrite)){
	   wxLogMessage("Failed to paste");
	   return;
	}
	SetStatus("The clipboard is now empty");
        RefreshFileList();
    }
    else{
     wxLogMessage("The target directory does not exsit");
    }

    } catch (const std::exception& e) {
        wxLogError("Error in OnPaste: %s", e.what());
    }		    
}

/*
 *Name: OnRefresh
 *Description: This function refreshes the file list for the curent directory in the UI
 *Parameters: wxCommandEvent & event, is the event that occured
 *Return: None
 * */
void Manager_Frame::OnRefresh(wxCommandEvent & event){

    try{
    
    RefreshFileList();

    } catch (const std::exception& e) {
        wxLogError("Error in OnRefresh: %s", e.what());
    }
}


/*
 *Name: OnQuit
 *Description: This function exits out of the program when the quit button is selected or the shortcut is inputted
 *Parameters: wxCommandEvent & event, is the event that occured
 *Return: None
 * */
void Manager_Frame::OnQuit(wxCommandEvent & event){
    
    Close(true);
}

/*
 *Name: UpdateAddressBar
 *Description: This function updates the address bar in the UI to show the current directory
 *Parameters: None
 *Return: None
 * */
void Manager_Frame::UpdateAddressBar() {
    // Force the text bar to show the current path
    addressBar->SetValue(myFS.GetCurrentDirectory().string());


}
