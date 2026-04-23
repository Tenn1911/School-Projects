/*
*Author: Tennyson Hung
*Description: This file contains the function declarations for the Manager_Frame class.
*Date: 2026-02-03
 * */

#ifndef FILE_MANAGER_FRAME_H
#define FILE_MANAGER_FRAME_H

#include <wx/wx.h>
#include <wx/menu.h>
#include <wx/listctrl.h>
#include "FileSystem.h"

namespace fs = std::filesystem;

class Manager_Frame : public wxFrame	
{
public:	
  Manager_Frame();
  void CreateLayout(wxWindow* parent);
  void InitializeAddressBar(wxWindow* parent);
  void InitializeFileListView(wxWindow* parent);
  void RefreshFileList();
  fs::path GetSelectedFilePath();
  void InitializeStatusBar();
  void SetStatus(std::string status);
  void CreateMenu();
  bool hasSelected();
  void SetSelectedFilePath(fs::path filePath) { selectedFilePath = filePath; };
  wxTextCtrl* GetAddressBar() { return addressBar; };
  wxListCtrl* GetFileListView() { return fileListView; };
  ~Manager_Frame(){};

protected:
  void OnAddressBarEnter(wxCommandEvent& event);
  void OnDoubleClick(wxListEvent& event);
  void OnOpen(wxCommandEvent & event);
  void OnNew(wxCommandEvent & event);
  void OnRename(wxCommandEvent & event);
  void OnDelete(wxCommandEvent & event);
  void OnCopy(wxCommandEvent & event);
  void OnCut(wxCommandEvent & event);
  void OnPaste(wxCommandEvent & event);
  void OnRefresh(wxCommandEvent & event);
  void OnQuit(wxCommandEvent & event);

private:	

  enum { OPEN_ID = 1, NEW_ID = 2, RENAME_ID = 3, DELETE_ID = 4, COPY_ID = 5, CUT_ID = 6, 
PASTE_ID = 7, REFRESH_ID = 8, ADDRESSBAR_ID = 9, FILELISTVIEW_ID = 10};
    
    FileSystem myFS;
    wxTextCtrl *addressBar;
    wxListCtrl *fileListView;
    wxStatusBar *statusBar;
    std::string selectedFilePath;

    //helper function to set the address bar to the current path
    void UpdateAddressBar();   

};



		     

#endif
