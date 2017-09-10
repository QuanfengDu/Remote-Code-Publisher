WPF instruction:

----After open WPA window, there are three tabs on the top of the window.

***** all the source files send from client to server are stored under the Repository folder.
***** after dependency analysis, all html files stored under HTML folder.
***** files downloaded from the server stored under Result folder.

   ---File List: contains four buttons.

                 Connect:             Connect client to server.
                 Select Directory:    Select files from local path.
                 Send Files:          Send files being chosed.
                 Analysis Dependency: After choose files, ask the server analyze dependency
                 
             ****You should click the Connect button first to connect with sever and download
                 the Javascript and CSS fils. Then, select files by clicking Select Directory.
                 After that, sending files by clicking Send Files. Finally, analysis dependency 
                 by clicking Analysis Dependency button.
      
   ---Content:  contains three buttons.

                 Category:            After finish dependency analysis, you can click the Category 
                                      button to show the content in server.

                 Select To View       If you want to see all the files in one folder, please select
                 and Files Under      one folder and click Select To view button, then click Files 
                 Folder:              Under Folder button.

   ---HTML:     contains five buttons.
        
                Show all HTML:        By clicking Show all HTML button, you can see the name of all
                                      HTML files. 
 
                Show Selected Items:  If you want to down load and delete HTML files, you are supposed
                                      to select the your files and click Show Selectd Items.

                Down Load:            After select files, by clicking Down Load button, you can down load
                                      files on local path, which is the Result Folder.

                Delete Selected File: After Clicking this button, the selected html file will be deleted 
                                      in the server side. 
                 
                Show HTML:            By clicking this button, it will open a html index page.

Meet requirement 1: using Visual Studio 2015 and its C++ Windows console project,
                    also use Windows Presentation Foundation (WPF) to provide a required
                    client Graphical User Interface (GUI)
                 2: use the C++ standard library's streams for all console I/O and new and delete for all heap-based memory management.
                 3: Provide the Repository program
                 4: After publish the html file, it meets all the requirement in project 3
                 5: Client can send files to sever after clicking the send file button, see html in the sever side
                    and download, delete files in the sever side, also can open a html page by clicking show html button,
                    meet the requirement 4
                 6: Using socket to communicate
                 7. Support HTTP message
                 8. Support sending stream of bytes
              





                 
                              
      
      