#include "videoComparisonViewer.hpp"

VideoComparisonViewer::VideoComparisonViewer(rapidjson::Document* settings, MainWindow* mainFrame, wxString projectDirectory)
	: wxFrame(NULL, wxID_ANY, "Video Comparison Viewer", wxDefaultPosition, wxSize(300, 200)) {
	// https://www.youtube.com/watch?v=su61pXgmJcw
	mainSettings = settings;
	parentWindow = mainFrame;
	projectDir   = projectDirectory;

	ffms2Errinfo.Buffer     = ffms2Errmsg;
	ffms2Errinfo.BufferSize = sizeof(ffms2Errmsg);
	ffms2Errinfo.ErrorType  = FFMS_ERROR_SUCCESS;
	ffms2Errinfo.SubType    = FFMS_ERROR_SUCCESS;

	mainSizer = new wxBoxSizer(wxVERTICAL);

	urlInput         = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_CENTRE);
	videoFormatsList = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE);

	inputSizer  = new wxBoxSizer(wxHORIZONTAL);
	frameSelect = new wxSpinCtrl(this, wxID_ANY, "Select Frame", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	frameSlider = new wxSlider(this, wxID_ANY, 0, 0, 10, wxDefaultPosition, wxDefaultSize, wxSL_LABELS | wxSL_HORIZONTAL);

	frameSelect->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &VideoComparisonViewer::frameChosenSpin, this);
	frameSlider->Bind(wxEVT_SLIDER, &VideoComparisonViewer::frameChosenSlider, this);

	inputSizer->Add(frameSelect, 1);
	inputSizer->Add(frameSlider, 1);

	consoleLog = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);

	urlInput->Bind(wxEVT_TEXT_ENTER, &VideoComparisonViewer::displayVideoFormats, this);
	videoFormatsList->Bind(wxEVT_LISTBOX, &VideoComparisonViewer::onFormatSelection, this);
	Bind(wxEVT_END_PROCESS, &VideoComparisonViewer::onCommandDone, this);

	videoCanvas = new DrawingCanvasBitmap(this, wxSize(1, 1));

	mainSizer->Add(urlInput, 0, wxEXPAND | wxALL);
	mainSizer->Add(videoFormatsList, 0, wxEXPAND | wxALL);
	mainSizer->Add(consoleLog, 0, wxEXPAND | wxALL);
	mainSizer->Add(videoCanvas, 0, wxSHAPED | wxEXPAND | wxALIGN_CENTER_HORIZONTAL);
	mainSizer->Add(inputSizer, 0, wxEXPAND | wxALL);

	// Hide by default
	videoFormatsList->Show(false);
	consoleLog->Show(true);
	videoCanvas->Show(false);
	Refresh();

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);
	Layout();
	Fit();
	Center(wxBOTH);

	Layout();
}

// clang-format off
BEGIN_EVENT_TABLE(VideoComparisonViewer, wxFrame)
	EVT_IDLE(VideoComparisonViewer::onIdle)
END_EVENT_TABLE()
// clang-format on

void VideoComparisonViewer::onIdle(wxIdleEvent& event) {
	// Check command output
	if(currentRunningCommand != RUNNING_COMMAND::NO_COMMAND) {
		wxInputStream* inputStream = commandProcess->GetInputStream();
		if(inputStream->CanRead()) {
			consoleLog->Show(true);
			Refresh();

			// https://forums.wxwidgets.org/viewtopic.php?t=24390#p104181

			char buffer[BUFSIZE];
			wxString text;
			inputStream->Read(buffer, BUFSIZE - 1);
			std::size_t count = inputStream->LastRead();
			if(count > 0) {
				text.Append(buffer, count);
			}

			consoleLog->AppendText(text);
		}
	}
}

void VideoComparisonViewer::onClose(wxCloseEvent& event) {
	// Help MainWindow delete this window
	for(std::size_t i = 0; i < parentWindow->videoComparisonViewers.size(); i++) {
		// If this is the window, remove it from the list
		if(parentWindow->videoComparisonViewers[i] == this) {
			parentWindow->videoComparisonViewers.erase(parentWindow->videoComparisonViewers.begin() + i);
			break;
		}
	}

	FFMS_DestroyVideoSource(videosource);

	Destroy();
}

void VideoComparisonViewer::onCommandDone(wxProcessEvent& event) {
	if(currentRunningCommand == RUNNING_COMMAND::DOWNLOAD_VIDEO) {
		// Finished downloading video
		currentRunningCommand = RUNNING_COMMAND::NO_COMMAND;
		delete commandProcess;
		commandProcess = nullptr;

		parseVideo();
	}
}

void VideoComparisonViewer::displayVideoFormats(wxCommandEvent& event) {
	url = urlInput->GetLineText(0).ToStdString();
	// All these commands will block, which could be a problem for bad internet connections
	videoName = HELPERS::exec(("youtube-dl --get-filename -o \"%(title)s.%(ext)s\" " + url).c_str());
	if(videoName.find("is not a valid URL.") == std::string::npos) {
		// Replace spaces with underscores
		std::replace(videoName.begin(), videoName.end(), ' ', '_');
		formatsArray.clear();
		formatsMetadataArray.clear();
		// Due to this https://forums.wxwidgets.org/viewtopic.php?t=20321
		videoFormatsList->Deselect(videoFormatsList->GetSelection());
		videoFormatsList->Clear();
		// This will block, so hopefully the internet is good
		std::string commandString = "youtube-dl -j " + url;
		std::string jsonString    = HELPERS::exec(commandString.c_str());
		// Read data
		rapidjson::Document jsonData = HELPERS::getSettingsFromString(jsonString);
		// For now, loop through formats
		int i = 0;
		if(jsonData.IsObject()) {
			for(auto const& format : jsonData["formats"].GetArray()) {
				// Check thing.json for an example
				// If width and height are null, it's audio
				if(format["width"].IsInt() && format["height"].IsInt() && format["fps"].IsInt()) {
					// Tiny means it only supports audio, we want video
					int formatID               = strtol(format["format_id"].GetString(), nullptr, 10);
					int width                  = format["width"].GetInt();
					int height                 = format["height"].GetInt();
					int fps                    = format["fps"].GetInt();
					wxString formatItem        = wxString::Format("%dx%d, %d fps", width, height, fps);
					wxString compactFormatItem = wxString::Format("%dx%d-%d-", width, height, fps);
					videoFormatsList->InsertItems(1, &formatItem, i);

					formatsArray.push_back(formatID);
					formatsMetadataArray.push_back(compactFormatItem.ToStdString());
					i++;
				}
			}
		} else {
			// Also not a valid URL
			wxMessageDialog urlInvalidDialog(this, "This URL is invalid", "Invalid URL", wxOK | wxICON_ERROR);
			urlInvalidDialog.ShowModal();
			return;
		}

		videoFormatsList->Show(true);
		Refresh();
	} else {
		// Not a valid URL
		wxMessageDialog urlInvalidDialog(this, "This URL is invalid", "Invalid URL", wxOK | wxICON_ERROR);
		urlInvalidDialog.ShowModal();
	}
}

void VideoComparisonViewer::onFormatSelection(wxCommandEvent& event) {
	if(videoExists) {
		FFMS_DestroyVideoSource(videosource);
		videoExists = false;
	}

	int selectedFormat = formatsArray[event.GetInt()];
	fullVideoPath      = projectDir.ToStdString() + "/videos/" + formatsMetadataArray[event.GetInt()] + videoName;
	// Streaming download from youtube-dl
	commandProcess = new wxProcess(this);
	commandProcess->Redirect();

	wxString commandString = wxString::Format("youtube-dl -f %d -o %s %s", selectedFormat, wxString::FromUTF8(fullVideoPath), url);

	// This will run and the progress will be seen in console
	currentRunningCommand = RUNNING_COMMAND::DOWNLOAD_VIDEO;
	wxExecute(commandString, wxEXEC_ASYNC | wxEXEC_SHOW_CONSOLE, commandProcess);
}

void VideoComparisonViewer::parseVideo() {
	// https://github.com/FFMS/ffms2/blob/master/doc/ffms2-api.md
	FFMS_Indexer* videoIndexer = FFMS_CreateIndexer(fullVideoPath.c_str(), &ffms2Errinfo);
	if(videoIndexer == NULL) {
		printFfms2Error();
		return;
	}
	// This is helpful for cpp callbacks
	// https://stackoverflow.com/a/29817048/9329945
	FFMS_SetProgressCallback(videoIndexer, &VideoComparisonViewer::onIndexingProgress, this);

	FFMS_Index* index = FFMS_DoIndexing2(videoIndexer, FFMS_IEH_ABORT, &ffms2Errinfo);
	if(index == NULL) {
		printFfms2Error();
		return;
	}

	int trackno = FFMS_GetFirstTrackOfType(index, FFMS_TYPE_VIDEO, &ffms2Errinfo);
	if(trackno < 0) {
		printFfms2Error();
		return;
	}

	videosource = FFMS_CreateVideoSource(fullVideoPath.c_str(), trackno, index, 1, FFMS_SEEK_NORMAL, &ffms2Errinfo);
	if(videosource == NULL) {
		printFfms2Error();
		return;
	}

	FFMS_DestroyIndex(index);

	videoprops                  = FFMS_GetVideoProperties(videosource);
	const FFMS_Frame* propframe = FFMS_GetFrame(videosource, 0, &ffms2Errinfo);

	videoDimensions.SetWidth(propframe->EncodedWidth);
	videoDimensions.SetHeight(propframe->EncodedHeight);
	videoCanvas->SetSize(videoDimensions);

	int pixfmts[2];
	pixfmts[0] = FFMS_GetPixFmt("rgb24");
	pixfmts[1] = -1;

	if(FFMS_SetOutputFormatV2(videosource, pixfmts, propframe->EncodedWidth, propframe->EncodedHeight, FFMS_RESIZER_BICUBIC, &ffms2Errinfo)) {
		printFfms2Error();
		return;
	}

	videoExists = true;
	consoleLog->Show(false);
	Refresh();

	frameSelect->SetRange(0, videoprops->NumFrames);
	frameSlider->SetRange(0, videoprops->NumFrames);

	frameSelect->SetValue(0);
	frameSlider->SetValue(0);

	drawFrame(0);
}

void VideoComparisonViewer::drawFrame(int frame) {
	// https://www.youtube.com/watch?v=su61pXgmJcw
	currentFrame = frame;

	consoleLog->Show(false);
	const FFMS_Frame* curframe = FFMS_GetFrame(videosource, frame, &ffms2Errinfo);
	if(curframe == NULL) {
		printFfms2Error();
		return;
	}

	// In the RGB format, the data is always in the first plane
	uint8_t* data = curframe->Data[0];
	int linesize  = curframe->Linesize[0];

	wxBitmap* videoFrame = new wxBitmap(videoDimensions, 24);
	wxNativePixelData nativePixelData(*videoFrame);
	nativePixelData.GetPixels();

	wxNativePixelData::Iterator p(nativePixelData);

	for(int y = 0; y < videoDimensions.GetHeight(); y++) {
		wxNativePixelData::Iterator rowStart = p;

		for(int x = 0; x < videoDimensions.GetWidth(); x++) {
			int start = y * linesize + x * 3;
			p.Red()   = data[start];
			p.Green() = data[start + 1];
			p.Blue()  = data[start + 2];
		}

		p = rowStart;
		p.OffsetY(nativePixelData, 1);
	}

	// The canvas will consume the bitmap
	videoCanvas->setBitmap(videoFrame);

	videoCanvas->Show(true);
	Refresh();
}

void VideoComparisonViewer::frameChosenSpin(wxSpinEvent& event) {
	int value = frameSelect->GetValue();
	if(videoExists) {
		drawFrame(value);
	}
	frameSlider->SetValue(value);
}
void VideoComparisonViewer::frameChosenSlider(wxCommandEvent& event) {
	int value = frameSlider->GetValue();
	if(videoExists) {
		drawFrame(value);
	}
	frameSelect->SetValue(value);
}

void VideoComparisonViewer::seekRelative(int relativeFrame) {
	// This is given the change that has passed, not the actual frame
	// Passed by DataProcessing
	if(videoExists) {
		currentFrame += relativeFrame;

		if(currentFrame > -1 && currentFrame < videoprops->NumFrames) {
			drawFrame(currentFrame);
		}
	}
}