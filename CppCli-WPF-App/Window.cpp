///////////////////////////////////////////////////////////////////////
// Window.pp                                                         //
// ver 3.9                                                           //
// Language:    C++, Visual Studio 2015                              //
// Platform:    Dell XPS 13, Windows 10                              //
// Application:  Object Oriented Design                              //
// Author:      Quanfeng Du, Syracuse University,                    //
//              qdu101@syr.edu                                       //
///////////////////////////////////////////////////////////////////////
/*
*  To run as a Windows Application:
*  - Set Project Properties > Linker > System > Subsystem to Windows
*  - Comment out int main(...) at bottom
*  - Uncomment int _stdcall WinMain() at bottom
*  To run as a Console Application:
*  - Set Project Properties > Linker > System > Subsytem to Console
*  - Uncomment int main(...) at bottom
*  - Comment out int _stdcall WinMain() at bottom
*/
#include "Window.h"
#include<sstream>
#include<fstream>

using namespace CppCliWindows;

//Set window
WPFCppCliDemo::WPFCppCliDemo()
{
  ObjectFactory* pObjFact = new ObjectFactory;
  pSendr_ = pObjFact->createSendr();
  pRecvr_ = pObjFact->createRecvr();
  pChann_ = pObjFact->createMockChannel(pSendr_, pRecvr_);
  pChann_->start();
  delete pObjFact;
  // client's receive thread
  recvThread = gcnew Thread(gcnew ThreadStart(this, &WPFCppCliDemo::getMessage));
  recvThread->Start();
  // set event handlers
  this->Loaded += 
    gcnew System::Windows::RoutedEventHandler(this, &WPFCppCliDemo::OnLoaded);
  this->Closing += 
    gcnew CancelEventHandler(this, &WPFCppCliDemo::Unloading);
  // set Window properties
  this->Title = "WPF C++/CLI Demo";
  this->Width = 800;
  this->Height = 600;
  // attach dock panel to Window
  this->Content = hDockPanel;
  hDockPanel->Children->Add(hStatusBar);
  hDockPanel->SetDock(hStatusBar, Dock::Bottom);
  hDockPanel->Children->Add(hGrid);
  // setup Window controls and views
  setbuttonclick();
  setUpTabControl();
  setUpStatusBar();
  setUpSendMessageView();
  setUpFileListView();
  setUpConnectionView();
  setUpHTMLView();
}

//Set button click happening 
void WPFCppCliDemo::setbuttonclick()
{
	CategoryText->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::showcontent);
   hFolderBrowseButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::browseForFolder);
   hShowItemsButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::getItemsFromList);
   sendFilesButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::sendFiles);
   hSelectHTML->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::getItemFromHTMLList);
   hDownloadHTML->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::downloadhtml);
   hDeletHTML->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::deletefile);
   hShowAllHTML->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::showallhtml);
   selectcatButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::sfileuncat);
   ShowFilesButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::ShowFiles);
   connectButton->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::Connect);
   showHtml->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::ShowHtml);
   Dodep->Click += gcnew RoutedEventHandler(this, &WPFCppCliDemo::doDepend);
}

WPFCppCliDemo::~WPFCppCliDemo()
{
  delete pChann_;
  delete pSendr_;
  delete pRecvr_;
}

//construct the HttpMessage
HttpMessage WPFCppCliDemo::makeHttpMessage(size_t n, const EndPoint& ep)
{
	HttpMessage msg;
	HttpMessage::Attribute attrib;
	EndPoint myEndPoint = "localhost:8081";
	switch (n)
	{
	case 1:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("POST", "Message"));
		msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));
		msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));
		msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));
		break;
	default:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("Error", "unknown message type"));
	}
	return msg;
}

//Set status button
void WPFCppCliDemo::setUpStatusBar()
{
  hStatusBar->Items->Add(hStatusBarItem);
  hStatus->Text = "very important messages will appear here";
  //status->FontWeight = FontWeights::Bold;
  hStatusBarItem->Content = hStatus;
  hStatusBar->Padding = Thickness(10, 2, 10, 2);
}

//Set up Tab happening
void WPFCppCliDemo::setUpTabControl()
{
  hGrid->Children->Add(hTabControl);
  Category->Header = "Content";
  hFileListTab->Header = "File List";
  hHTMLTab->Header = "HTML";
  hConnectTab->Header = "Connect";
  hTabControl->Items->Add(hFileListTab);
  hTabControl->Items->Add(Category);
  hTabControl->Items->Add(hHTMLTab);
}

//Set property for TextBlock
void WPFCppCliDemo::setTextBlockProperties()
{
  RowDefinition^ hRow1Def = gcnew RowDefinition();
  hSendMessageGrid->RowDefinitions->Add(hRow1Def);
  Border^ hBorder1 = gcnew Border();
  hBorder1->BorderThickness = Thickness(1);
  hBorder1->BorderBrush = Brushes::Black;
  hBorder1->Child = hListBoxCat;
  hListBoxCat->Padding = Thickness(15);
  hListBoxCat->FontFamily = gcnew Windows::Media::FontFamily("Tahoma");
  hListBoxCat->FontWeight = FontWeights::Bold;
  hListBoxCat->FontSize = 16;
  hScrollViewer1->VerticalScrollBarVisibility = ScrollBarVisibility::Auto;
  hScrollViewer1->Content = hBorder1;
  hSendMessageGrid->SetRow(hScrollViewer1, 0);
  hSendMessageGrid->Children->Add(hScrollViewer1);
}

//Set property for Buttons 
void WPFCppCliDemo::setButtonsProperties()
{
  RowDefinition^ hRow2Def = gcnew RowDefinition();
  hRow2Def->Height = GridLength(75);
  hSendMessageGrid->RowDefinitions->Add(hRow2Def);
  CategoryText->Content = "Category";
  Border^ hBorder2 = gcnew Border();
  hBorder2->Width = 120;
  hBorder2->Height = 30;
  hBorder2->BorderThickness = Thickness(1);
  hBorder2->BorderBrush = Brushes::Black;
  selectcatButton->Content = "Select To view";
  hBorder2->Child = CategoryText;
  Border^ hBorder3 = gcnew Border();
  hBorder3->Width = 120;
  hBorder3->Height = 30;
  hBorder3->BorderThickness = Thickness(1);
  hBorder3->BorderBrush = Brushes::Black;
  hBorder3->Child = selectcatButton;
  ShowFilesButton->Content = "Files Under Folder";
  Border^ hBorder4 = gcnew Border();
  hBorder4->Width = 120;
  hBorder4->Height = 30;
  hBorder4->BorderThickness = Thickness(1);
  hBorder4->BorderBrush = Brushes::Black;
  hBorder4->Child = ShowFilesButton;
  hStackPanel1->Children->Add(hBorder2);
  TextBlock^ hSpacer = gcnew TextBlock();
  hSpacer->Width = 10;
  hStackPanel1->Children->Add(hSpacer);
  TextBlock^ hSpacer1 = gcnew TextBlock();
  hSpacer1->Width = 10;
  hStackPanel1->Children->Add(hBorder3);
  hStackPanel1->Children->Add(hSpacer1);
  hStackPanel1->Children->Add(hBorder4);
  hStackPanel1->Orientation = Orientation::Horizontal;
  hStackPanel1->HorizontalAlignment = System::Windows::HorizontalAlignment::Center;
  hSendMessageGrid->SetRow(hStackPanel1, 1);
  hSendMessageGrid->Children->Add(hStackPanel1);
}

//Set view for send message tab
void WPFCppCliDemo::setUpSendMessageView()
{
  Console::Write("\n  setting up sendMessage view");
  hSendMessageGrid->Margin = Thickness(20);
  Category->Content = hSendMessageGrid;

  setTextBlockProperties();
  setButtonsProperties();
}

//Convert system string to std string 
std::string WPFCppCliDemo::toStdString(String^ pStr)
{
  std::string dst;
  for (int i = 0; i < pStr->Length; ++i)
    dst += (char)pStr[i];
  return dst;
}

void WPFCppCliDemo::sendMessage(Object^ obj, RoutedEventArgs^ args)
{
  Console::Write("\n  sent message");
  hStatus->Text = "Sent message";
}

//send do dependency message
void WPFCppCliDemo::doDepend(Object^ sender, RoutedEventArgs^ args)
{
	HttpMessage msg_ = makewholeMessage(1, "", "localhost::8080");
	msg_.addAttribute(HttpMessage::Attribute("command", "Dodepend"));
	pSendr_->postMessage(msg_);
}

//construct send file message and enq this message
void WPFCppCliDemo::sendFiles(Object^ obj, RoutedEventArgs^ args)
{
	for (int i=0; i<files->Length; ++i)
	{
		HttpMessage msg;
		HttpMessage::Attribute attrib;
		msg.addAttribute(HttpMessage::attribute("Command", "Upload"));
		msg.addAttribute(HttpMessage::attribute("ToAddr", "8080"));
		msg.addAttribute(HttpMessage::attribute("FromAddr", "8081"));
		msg.addAttribute(HttpMessage::attribute("Mode", "OneWay"));
		msg.addAttribute(HttpMessage::attribute("command", "sendsourcefile"));
		msg.addAttribute(HttpMessage::attribute("file", toStdString(files[i])));
		pSendr_->postMessage(msg);
		hStatus->Text = "Sent files";
	}
}

//send do connect message
void WPFCppCliDemo::Connect(Object^ sender, RoutedEventArgs^ args)
{
	HttpMessage msg_ = makewholeMessage(1, "", "localhost::8080");
	msg_.addAttribute(HttpMessage::Attribute("command", "connect"));
	pSendr_->postMessage(msg_);
}

//Convert std string to system string 
String^ WPFCppCliDemo::toSystemString(std::string& str)
{
  StringBuilder^ pStr = gcnew StringBuilder();
  for (size_t i = 0; i < str.size(); ++i)
    pStr->Append((Char)str[i]);
  return pStr->ToString();
}

//addtext in box
void WPFCppCliDemo::addText(String^ msg)
{
  //hTextBlock1->Text += msg + "\n\n";
}

//add text in html list
void WPFCppCliDemo::addhtml(String^ msg)
{
	hHTMLList->Items->Add(msg);
}

//add text in category list
void WPFCppCliDemo::addCategory(String^ msg)
{
	hListBoxCat->Items->Add(msg);
}

//function to show message in different list 
void WPFCppCliDemo::getMessage()
{
   //recvThread runs this function
  while (true)
  {
    HttpMessage msg = pRecvr_->getMessage();
    //HttpMessage msg2= pRecvr_->getMessage();
	std::string command = msg.findValue("command");
	if (command == "View all HTML")
	{
		View_all_HTML(msg);
	} 
	if (command == "category")
	{
		Category_(msg);
	}
	if (command == "Details")
	{
		Details_(msg);
	}
  }
}

//send view all html message
void WPFCppCliDemo::View_all_HTML(HttpMessage msg)
{
	std::string requestfilename = msg.bodyString();
	std::istringstream f(requestfilename);
	std::string elem;
	while (std::getline(f, elem))
	{
		String^ smes = toSystemString(elem);
		array<String^>^ args = gcnew array<String^>(1);
		args[0] = smes;
		Action<String^>^ act = gcnew Action<String^>(this, &WPFCppCliDemo::addhtml);
		Dispatcher->Invoke(act, args);
	}
}

//send view category message
void WPFCppCliDemo::Category_(HttpMessage msg)
{
	std::string requestcategory = msg.bodyString();
	std::istringstream f(requestcategory);
	std::string elem;
	while (std::getline(f, elem))
	{
		if (elem == "." || elem == "..")
			continue;
		String^ smes = toSystemString(elem);
		array<String^>^ args = gcnew array<String^>(1);
		args[0] = smes;
		Action<String^>^ act = gcnew Action<String^>(this, &WPFCppCliDemo::addCategory);
		Dispatcher->Invoke(act, args);
	}
}

//send view details message
void WPFCppCliDemo::Details_(HttpMessage msg)
{
	std::string t = " ";
	String^ smes = toSystemString(t);
	array<String^>^ args = gcnew array<String^>(1);
	args[0] = smes;
	Action<String^>^ act = gcnew Action<String^>(this, &WPFCppCliDemo::addfilrs);
	Dispatcher->Invoke(act, args);
	std::string requestcategory = msg.bodyString();
	std::istringstream f(requestcategory);
	std::string elem;
	while (std::getline(f, elem))
	{
		if (elem == "." || elem == "..")
			continue;
		String^ smes = toSystemString(elem);
		array<String^>^ args = gcnew array<String^>(1);
		args[0] = smes;
		Action<String^>^ act = gcnew Action<String^>(this, &WPFCppCliDemo::addCategory);
		Dispatcher->Invoke(act, args);
	}
}

//add files to html list box
void WPFCppCliDemo::addfilrs(String^ msg)
{
	hListBoxCat->Items->Clear();
}

//function to get items from html list
void WPFCppCliDemo::getItemsFromList(Object^ sender, RoutedEventArgs^ args)
{
	int index = 0;
	int count = hListBox->SelectedItems->Count;
	hStatus->Text = "Show Selected Items";
	array<System::String^>^ items = gcnew array<String^>(count);
	if (count > 0) {
		for each (String^ item in hListBox->SelectedItems)
		{
			items[index++] = item;
		}
	}
	hListBox->Items->Clear();
	if (count > 0) {
		for each (String^ item in items)
		{
			hListBox->Items->Add(item);
		}
	}
}


//function to set up file list view
void WPFCppCliDemo::setUpFileListView()
{
  hFileListGrid->Margin = Thickness(20);
  hFileListTab->Content = hFileListGrid;
  RowDefinition^ hRow1Def = gcnew RowDefinition();
  hFileListGrid->RowDefinitions->Add(hRow1Def);
  Border^ hBorder1 = gcnew Border();
  hBorder1->BorderThickness = Thickness(1);
  hBorder1->BorderBrush = Brushes::Black;
  hListBox->SelectionMode = SelectionMode::Multiple;
  hBorder1->Child = hListBox;
  hFileListGrid->SetRow(hBorder1, 0);
  hFileListGrid->Children->Add(hBorder1);
  RowDefinition^ hRow2Def = gcnew RowDefinition();
  hRow2Def->Height = GridLength(55);
  RowDefinition^ hRow2Def2 = gcnew RowDefinition();
  hRow2Def2->Height = GridLength(55);
  RowDefinition^ hRow2Def3 = gcnew RowDefinition();
  hRow2Def3->Height = GridLength(55);
  RowDefinition^ hRow2Def4 = gcnew RowDefinition();
  hRow2Def4->Height = GridLength(55);
  hFileListGrid->RowDefinitions->Add(hRow2Def);
  hFileListGrid->RowDefinitions->Add(hRow2Def2);
  hFileListGrid->RowDefinitions->Add(hRow2Def3);
  hFileListGrid->RowDefinitions->Add(hRow2Def4);
  sethFolderBrowseButton();
  hFileListGrid->SetRow(hFolderBrowseButton, 2);
  hFileListGrid->Children->Add(hFolderBrowseButton);
  hFolderBrowserDialog->ShowNewFolderButton = false;
  hFolderBrowserDialog->SelectedPath = System::IO::Directory::GetCurrentDirectory();
  setsendFilesButton();
  hFileListGrid->SetRow(sendFilesButton, 3);
  hFileListGrid->Children->Add(sendFilesButton);
  setDodep();
  hFileListGrid->SetRow(Dodep, 4);
  hFileListGrid->Children->Add(Dodep);
  setconnectButton();
  hFileListGrid->SetRow(connectButton, 1);
  hFileListGrid->Children->Add(connectButton);
}

//set folder button 
void WPFCppCliDemo::sethFolderBrowseButton()
{
	hFolderBrowseButton->Content = "Select Directory";
	hFolderBrowseButton->Height = 30;
	hFolderBrowseButton->Width = 120;
	hFolderBrowseButton->BorderThickness = Thickness(2);
	hFolderBrowseButton->BorderBrush = Brushes::Black;
}

//set send file button
void WPFCppCliDemo::setsendFilesButton()
{
	sendFilesButton->Content = "Send Files";
	sendFilesButton->Height = 30;
	sendFilesButton->Width = 120;
	sendFilesButton->BorderThickness = Thickness(2);
	sendFilesButton->BorderBrush = Brushes::Black;
}

//set do dependency analysis button
void WPFCppCliDemo::setDodep()
{
	Dodep->Content = "Analysis Dependency";
	Dodep->Height = 30;
	Dodep->Width = 120;
	Dodep->BorderThickness = Thickness(2);
	Dodep->BorderBrush = Brushes::Black;
}

//set connectiong button
void WPFCppCliDemo::setconnectButton()
{
	connectButton->Content = "Connect";
	connectButton->Height = 30;
	connectButton->Width = 120;
	connectButton->BorderThickness = Thickness(2);
	connectButton->BorderBrush = Brushes::Black;
}

//function to set browse folder
void WPFCppCliDemo::browseForFolder(Object^ sender, RoutedEventArgs^ args)
{
  std::cout << "\n  Browsing for folder";
  hListBox->Items->Clear();
  System::Windows::Forms::DialogResult result;
  result = hFolderBrowserDialog->ShowDialog();
  if (result == System::Windows::Forms::DialogResult::OK)
  {
    String^ path = hFolderBrowserDialog->SelectedPath;
    std::cout << "\n  opening folder \"" << toStdString(path) << "\"";
    files = System::IO::Directory::GetFiles(path, L"*.*");
    for (int i = 0; i < files->Length; ++i)
      hListBox->Items->Add(files[i]);
    array<String^>^ dirs = System::IO::Directory::GetDirectories(path);
    for (int i = 0; i < dirs->Length; ++i)
      hListBox->Items->Add(L"<> " + dirs[i]);
  }
}

//function to show content 
void WPFCppCliDemo::showcontent(Object^ sender, RoutedEventArgs^ args)
{
	hListBoxCat->Items->Clear();
	HttpMessage msg_ = makewholeMessage(1, "", "localhost::8080");
	msg_.addAttribute(HttpMessage::Attribute("command", "category"));
	pSendr_->postMessage(msg_);
}

//function to select category
void WPFCppCliDemo::sfileuncat(Object^ sender, RoutedEventArgs^ args)
{
	int index = 0;
	int count = hListBoxCat->SelectedItems->Count;
	hStatus->Text = "Show Selected category";
	items_n = gcnew array<String^>(count);
	if (count > 0)
	{
		for each (String^ item in hListBoxCat->SelectedItems)
		{
			items_n[index++] = item;
		}
	}
	hListBoxCat->Items->Clear();
	if (count > 0)
	{
		for each (String^ item in items_n)
		{
			hListBoxCat->Items->Add(item);
		}
	}
}

////function to show files
void WPFCppCliDemo::ShowFiles(Object^ sender, RoutedEventArgs^ args)
{
		std::string all;
		for each(String^ item in items_n)
		{
			std::string item_ = toStdString(item);
			all = item_;
		}
		HttpMessage msg_ = makewholeMessage(1, "", "localhost::8080");
		msg_.addAttribute(HttpMessage::Attribute("command", "Details"));
		msg_.addAttribute(HttpMessage::Attribute("Files_under_F", all));
		std::cout << "\n    test " << msg_.toString();
		pSendr_->postMessage(msg_);
	
}

//function to set HTML tab view
void WPFCppCliDemo::setUpHTMLView()
{
	Console::Write("\n  setting up FileList view");
	hHTMLListGrid->Margin = Thickness(20);
	hHTMLTab->Content = hHTMLListGrid;
	RowDefinition^ hRow1Def = gcnew RowDefinition();
	hHTMLListGrid->RowDefinitions->Add(hRow1Def);
	Border^ hBorder1 = gcnew Border();
	hBorder1->BorderThickness = Thickness(1);
	hBorder1->BorderBrush = Brushes::Black;
	hHTMLList->SelectionMode = SelectionMode::Multiple;
	hBorder1->Child = hHTMLList;
	hHTMLListGrid->SetRow(hBorder1, 0);
	hHTMLListGrid->Children->Add(hBorder1);
	RowDefinition^ hRow2Def = gcnew RowDefinition();
	hRow2Def->Height = GridLength(30);
	RowDefinition^ hRow2Def2 = gcnew RowDefinition();
	hRow2Def2->Height = GridLength(30);
	RowDefinition^ hRow2Def3 = gcnew RowDefinition();
	hRow2Def3->Height = GridLength(30);
	RowDefinition^ hRow2Def4 = gcnew RowDefinition();
	hRow2Def4->Height = GridLength(30);
	RowDefinition^ hRow2Def5 = gcnew RowDefinition();
	hRow2Def5->Height = GridLength(30);
	hHTMLListGrid->RowDefinitions->Add(hRow2Def);
	hHTMLListGrid->RowDefinitions->Add(hRow2Def2);
	hHTMLListGrid->RowDefinitions->Add(hRow2Def3);
	hHTMLListGrid->RowDefinitions->Add(hRow2Def4);
	hHTMLListGrid->RowDefinitions->Add(hRow2Def5);
	sethShowAllHTML();
	hHTMLListGrid->SetRow(hShowAllHTML, 1);
	hHTMLListGrid->Children->Add(hShowAllHTML);
	sethSelectHTML();
	hHTMLListGrid->SetRow(hSelectHTML, 2);
	hHTMLListGrid->Children->Add(hSelectHTML);
	sethDownloadHTML();
	hHTMLListGrid->SetRow(hDownloadHTML, 3);
	hHTMLListGrid->Children->Add(hDownloadHTML);
	sethDeletHTML();
    hHTMLListGrid->SetRow(hDeletHTML, 4);
	hHTMLListGrid->Children->Add(hDeletHTML);
	setshowHtml();
	hHTMLListGrid->SetRow(showHtml, 5);
	hHTMLListGrid->Children->Add(showHtml);
}

//set show all html button
void WPFCppCliDemo::sethShowAllHTML()
{
	hShowAllHTML->Content = "Show all HTML";
	hShowAllHTML->Height = 30;
	hShowAllHTML->Width = 120;
	hShowAllHTML->BorderThickness = Thickness(2);
	hShowAllHTML->BorderBrush = Brushes::Black;
}
//set select html button
void WPFCppCliDemo::sethSelectHTML()
{
	hSelectHTML->Content = "Show Selected Items";
	hSelectHTML->Height = 30;
	hSelectHTML->Width = 120;
	hSelectHTML->BorderThickness = Thickness(2);
	hSelectHTML->BorderBrush = Brushes::Black;
}

//set download button
void WPFCppCliDemo::sethDownloadHTML()
{
	hDownloadHTML->Content = "Down Load";
	hDownloadHTML->Height = 30;
	hDownloadHTML->Width = 120;
	hDownloadHTML->BorderThickness = Thickness(2);
	hDownloadHTML->BorderBrush = Brushes::Black;
}

//set delete html button
void WPFCppCliDemo::sethDeletHTML()
{
	hDeletHTML->Content = "Delete Selected File";
	hDeletHTML->Height = 30;
	hDeletHTML->Width = 120;
	hDeletHTML->BorderThickness = Thickness(2);
	hDeletHTML->BorderBrush = Brushes::Black;
}

//set show html button
void WPFCppCliDemo::setshowHtml()
{
	showHtml->Content = "Show HTML";
	showHtml->Height = 30;
	showHtml->Width = 120;
	showHtml->BorderThickness = Thickness(2);
	showHtml->BorderBrush = Brushes::Black;
}

//set connect button 
void WPFCppCliDemo::setUpConnectionView()
{
  Console::Write("\n  setting up Connection view");
}

//used to get items from html list
void WPFCppCliDemo::getItemFromHTMLList(Object^ sender, RoutedEventArgs^ args)
{
	int index = 0;
	int count = hHTMLList->SelectedItems->Count;
	hStatus->Text = "Show Selected HTML";
	items_n = gcnew array<String^>(count);
	if (count > 0) 
	{
		for each (String^ item in hHTMLList->SelectedItems)
		{
			items_n[index++] = item;
		}
	}
	hHTMLList->Items->Clear();
	if (count > 0) 
	{
		for each (String^ item in items_n)
		{
			hHTMLList->Items->Add(item);
		}
	}
}

//used to make down load message and enq
void WPFCppCliDemo::downloadhtml(Object^ sender,RoutedEventArgs^ args)
{
	int count = items_n->Length;
	if (count > 0)
	{
		std::string allfilename;
		for each (String^ item in items_n)
		{
			std::string item_ = toStdString(item);
			allfilename = allfilename + item_ + ",";
		}
		HttpMessage msg_ = makewholeMessage(1,"","localhost::8080");
		msg_.addAttribute(HttpMessage::Attribute("command", "download"));
		msg_.addAttribute(HttpMessage::Attribute("FileDL", allfilename));
		pSendr_->postMessage(msg_);
	}
}

//used to delete files
void WPFCppCliDemo::deletefile(Object^ sender, RoutedEventArgs^ args)
{
	int count = items_n->Length;
	if (count > 0)
	{
		std::string allfilename;
		for each(String^ item in items_n)
		{
			std::string item_ = toStdString(item);
			allfilename = allfilename + item_ + ", ";
		}
		HttpMessage msg_ = makewholeMessage(1, "", "localhost::8080");
		msg_.addAttribute(HttpMessage::Attribute("command", "delete"));
		msg_.addAttribute(HttpMessage::Attribute("FileDL", allfilename));
		pSendr_->postMessage(msg_);
	}
}

//used to show HTML file name in html list
void WPFCppCliDemo::ShowHtml(Object^ sender, RoutedEventArgs^ args)
{
	std::string Command = "ShowHtml";
	HttpMessage msg = makeHttpMessage(1, "localhost::8080");
	msg.addAttribute(HttpMessage::Attribute("command", Command));
	pSendr_->postMessage(msg);

	std::string path = "..\\Result\\allfiles.html";
	system(path.c_str());
}

//used to construct HttpMessage
HttpMessage WPFCppCliDemo::makewholeMessage(size_t n, const std::string& body, const EndPoint& ep)
{
	HttpMessage msg;
	HttpMessage::Attribute attrib;
	EndPoint myEndPoint = "localhost:8081";  // ToDo: make this a member of the sender
											 // given to its constructor.
	switch (n)
	{
	case 1:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("POST", "Message"));
		msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));
		msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));
		msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));
		msg.addBody(body);
		break;
	default:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("Error", "unknown message type"));
	}
	return msg;
}

//used to show all html file name
void WPFCppCliDemo::showallhtml(Object^ sender, RoutedEventArgs^ args)
{
	std::cout << "\n show all html files";
	hStatus->Text = "Require all html files from server";
	hHTMLList->Items->Clear();
	std::string Command = "View all HTML";
	HttpMessage msg = makeHttpMessage(1, "localhost::8080");
	msg.addAttribute(HttpMessage::Attribute("command", Command));
	pSendr_->postMessage(msg);
	std::cout << "\n" << msg.toString();
}


void WPFCppCliDemo::OnLoaded(Object^ sender, RoutedEventArgs^ args)
{
  Console::Write("\n  Window loaded");
}
void WPFCppCliDemo::Unloading(Object^ sender, System::ComponentModel::CancelEventArgs^ args)
{
  Console::Write("\n  Window closing");
}

[STAThread]
//int _stdcall WinMain()
int main(array<System::String^>^ args)
{
  Console::WriteLine(L"\n Starting WPFCppCliDemo");

  Application^ app = gcnew Application();
  app->Run(gcnew WPFCppCliDemo());
  Console::WriteLine(L"\n\n");
}