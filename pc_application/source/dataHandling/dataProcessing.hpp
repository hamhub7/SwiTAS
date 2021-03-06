#pragma once

#ifdef __WXMSW__
#include <Windows.h>
#include <uxtheme.h>
#endif

#include <bitset>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <inttypes.h>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <wx/clipbrd.h>
#include <wx/grid.h>
#include <wx/imaglist.h>
#include <wx/itemattr.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <wx/wx.h>

#include "../sharedNetworkCode/networkInterface.hpp"
#include "buttonConstants.hpp"
#include "buttonData.hpp"

typedef std::shared_ptr<ControllerData> FrameData;
typedef std::vector<std::shared_ptr<std::vector<std::shared_ptr<SavestateHook>>>> AllPlayers;
typedef std::vector<std::shared_ptr<SavestateHook>> AllSavestateHookBlocks;
typedef std::shared_ptr<std::vector<FrameData>> BranchData;

class ButtonData;

class DataProcessing : public wxListCtrl {
	// clang-format on
private:
	// Vector storing inputs for current savestate hook
	// SavestateHookBlock inputsList;
	// Current input
	FrameData currentData;
	BranchData currentBranchData;
	// Button data instance (never changes)
	std::shared_ptr<ButtonData> buttonData;
	// Vector holding the savestate hook blocks
	// Is a vector of the players as well
	AllPlayers allPlayers;

	wxFileName projectStart;

	bool tethered = false;

	// Current frames (all relative to the start of the savestate hook block)
	// What you can edit
	FrameNum currentFrame = 0;
	// What is the state of the switch
	FrameNum currentRunFrame = 0;
	// What is the image you can see
	FrameNum currentImageFrame = 0;
	// Savestate hook number
	SavestateBlockNum currentSavestateHook = 0;

	uint8_t viewingPlayerIndex  = 0;
	uint16_t viewingBranchIndex = 0;

	wxImageList imageList;

	// Using callbacks for inputs
	std::function<void(uint8_t)> inputCallback;
	std::function<void(FrameNum)> selectedFrameCallbackVideoViewer;
	std::function<void(FrameNum, FrameNum)> viewableInputsCallback;
	std::function<void(FrameNum, FrameNum, FrameNum)> changingSelectedFrameCallback;
	std::function<void(uint8_t, uint8_t, bool)> playerInfoCallback;
	std::function<void(uint16_t, uint16_t, bool)> branchInfoCallback;

	// Network instance for sending to switch
	std::shared_ptr<CommunicateWithNetwork> networkInstance;
	// Main settings
	rapidjson::Document* mainSettings;

	// Mask color for transparency
	wxColour maskColor;

	// Map to get column
	std::unordered_map<Btn, uint8_t> buttonToColumn;
	std::unordered_map<wxChar, Btn> charToButton;

	// Probably not smart, but the current savestate hooks
	std::unordered_map<FrameNum, std::shared_ptr<Savestate>> savestates;
	// Universal item attributes for certain attributes
	std::unordered_map<uint8_t, wxItemAttr*> itemAttributes;

	virtual int OnGetItemColumnImage(long item, long column) const override;
	virtual wxString OnGetItemText(long item, long column) const override;
	virtual wxItemAttr* OnGetItemAttr(long item) const override;

	void setItemAttributes();

	void OnEraseBackground(wxEraseEvent& event);

	// Custom accelerator IDs
	int pasteInsertID;
	int pastePlaceID;
	int addFrameID;
	int add10FramesID;
	int removeFrameID;
	int frameAdvanceID;
	int savestateID;
	int mergeIntoMainBranchID;

	int insertPaste;
	bool placePaste;

	// Menu popup
	wxMenu editMenu;

	void onDropFiles(wxDropFilesEvent& event);

	void onRightClick(wxContextMenuEvent& event);
	void onSelect(wxListEvent& event);
	void onActivate(wxListEvent& event);
	void onCopy(wxCommandEvent& event);
	void onCut(wxCommandEvent& event);
	void onPaste(wxCommandEvent& event);
	void onInsertPaste(wxCommandEvent& event);
	void onPlacePaste(wxCommandEvent& event);

	void onAddFrame(wxCommandEvent& event);
	void onAdd10Frames(wxCommandEvent& event);
	void onRemoveFrame(wxCommandEvent& event);
	void onFrameAdvance(wxCommandEvent& event);
	void onAddSavestate(wxCommandEvent& event);
	void onMergeIntoMainBranch(wxCommandEvent& event);

public:
	static const int LIST_CTRL_ID = 1000;

	// All blocks loaded in by projectManager
	DataProcessing(rapidjson::Document* settings, std::shared_ptr<ButtonData> buttons, std::shared_ptr<CommunicateWithNetwork> communicateWithNetwork, wxWindow* parent);

	void setInputCallback(std::function<void(uint8_t)> callback);
	void setSelectedFrameCallbackVideoViewer(std::function<void(int)> callback);
	void setViewableInputsCallback(std::function<void(FrameNum, FrameNum)> callback);
	void setChangingSelectedFrameCallback(std::function<void(FrameNum, FrameNum, FrameNum)> callback);
	void setPlayerInfoCallback(std::function<void(uint8_t, uint8_t, bool)> callback);
	void setBranchInfoCallback(std::function<void(uint16_t, uint16_t, bool)> callback);
	void triggerCurrentFrameChanges();

	void sendAutoAdvance(uint8_t includeFramebuffer);

	std::string getExportedCurrentPlayer();
	void importFromFile(wxFileName importTarget);

	void setProjectStart(wxFileName start) {
		projectStart = start;
	}

	AllSavestateHookBlocks& getAllSavestateHookBlocks() {
		return *allPlayers[viewingPlayerIndex];
	}

	void setAllPlayers(AllPlayers players) {
		// Called by projectHandlerplayers when loading
		// It has to have at least one block with one input
		allPlayers.clear();
		for(auto& player : players) {
			allPlayers.push_back(player);
		}
		setPlayer(0);
	}

	AllPlayers& getAllPlayers() {
		return allPlayers;
	}

	uint8_t getCurrentPlayer() {
		return viewingPlayerIndex;
	}
	SavestateBlockNum getCurrentSavestateHook() {
		return currentSavestateHook;
	}
	FrameNum getCurrentFrame() {
		return currentFrame;
	}
	uint16_t getCurrentBranch() {
		return viewingBranchIndex;
	}
	SavestateBlockNum getNumOfSavestateHooks(uint8_t player) {
		return allPlayers[player]->size();
	}
	FrameNum getNumOfFramesInSavestateHook(SavestateBlockNum i, uint8_t player) {
		return allPlayers[player]->at(i)->inputs[viewingBranchIndex]->size();
	}

	wxFileName getFramebufferPath(uint8_t player, SavestateBlockNum savestateHookNum, BranchNum branch, FrameNum frame) {
		// The player does not matter in the path
		wxFileName framebufferFileName = projectStart;
		framebufferFileName.AppendDir("framebuffers");
		framebufferFileName.AppendDir(wxString::Format("savestate_block_%u", savestateHookNum));

		if(branch == 0) {
			// Main branch gets a special name
			framebufferFileName.AppendDir("branch_main");
		} else {
			framebufferFileName.AppendDir(wxString::Format("branch_%u", branch));
		}

		framebufferFileName.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

		framebufferFileName.SetName(wxString::Format("frame_%lu_screenshot", frame));
		framebufferFileName.SetExt("jpg");
		return framebufferFileName;
	}

	wxFileName getFramebufferPathForCurrent() {
		return getFramebufferPath(viewingPlayerIndex, currentSavestateHook, viewingBranchIndex, currentFrame);
	}

	wxFileName getFramebufferPathForSavestateHook(SavestateBlockNum index) {
		// Display the image linked with the savestate hook
		wxFileName framebufferFileName = projectStart;
		framebufferFileName.AppendDir("framebuffers");

		framebufferFileName.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

		wxString name = "savestate_block_%u_screenshot";
		framebufferFileName.SetName(wxString::Format(name, index));
		framebufferFileName.SetExt("jpg");
		return framebufferFileName;
	}

	wxFileName getFramebufferPathForCurrentFramebuf() {
		if(currentImageFrame == 0) {
			return getFramebufferPathForSavestateHook(currentSavestateHook);
		} else {
			return getFramebufferPath(viewingPlayerIndex, currentSavestateHook, viewingBranchIndex, currentImageFrame);
		}
	}

	void setTethered(bool flag) {
		tethered = flag;
	}

	void setCurrentFrame(FrameNum frameNum);

	void createSavestateHere();
	void runFrame(uint8_t forAutoFrame, uint8_t updateFramebuffer, uint8_t includeFramebuffer);

	// TODO cache this
	std::shared_ptr<std::vector<std::shared_ptr<ControllerData>>> getInputsList() const;

	std::shared_ptr<ControllerData> getControllerData(uint8_t player, SavestateBlockNum savestateHookNum, BranchNum branch, FrameNum frame) const {
		return allPlayers[player]->at(savestateHookNum)->inputs[branch]->at(frame);
	}

	wxRect getFirstItemRect();

	bool handleKeyboardInput(wxChar key);

	void onCacheHint(wxListEvent& event);

	void addNewSavestateHook(std::string dHash, wxBitmap* screenshot);
	void setSavestateHook(SavestateBlockNum index);
	void removeSavestateHook(SavestateBlockNum index);

	void addNewPlayer();
	void setPlayer(uint8_t playerIndex);
	void removePlayer(uint8_t playerIndex);
	void removeThisPlayer();
	void sendPlayerNum();

	void addNewBranch();
	void setBranch(uint16_t branchIndex);
	void removeBranch(uint8_t branchIndex);
	void removeThisBranch();
	uint16_t getNumBranches() {
		return allPlayers[viewingPlayerIndex]->at(currentSavestateHook)->inputs.size();
	}

	std::shared_ptr<ControllerData> getFrame(FrameNum frame) const;

	void scrollToSpecific(uint8_t player, SavestateBlockNum savestateHookNum, BranchNum branch, FrameNum frame);

	// New FANCY methods
	void triggerButton(Btn button);
	void modifyButton(FrameNum frame, Btn button, uint8_t isPressed);
	void toggleButton(FrameNum frame, Btn button);
	void clearAllButtons(FrameNum frame);
	uint8_t getButton(FrameNum frame, Btn button) const;
	uint8_t getButtonSpecific(FrameNum frame, Btn button, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const;
	uint8_t getButtonCurrent(Btn button) const;

	void setControllerDataForAutoRun(ControllerData controllerData);

	// This includes joysticks, accel, gyro, etc...
	void triggerNumberValues(ControllerNumberValues joystickId, int16_t value);
	void setNumberValues(FrameNum frame, ControllerNumberValues joystickId, int16_t value);
	int16_t getNumberValues(FrameNum frame, ControllerNumberValues joystickId) const;
	int16_t getNumberValuesSpecific(FrameNum frame, ControllerNumberValues joystickId, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const;
	int16_t getNumberValueCurrent(ControllerNumberValues joystickId) const;

	// Updates how the current frame looks on the UI
	// Also called when modifying anything of importance, like currentFrame
	void modifyCurrentFrameViews(FrameNum frame);

	void setFramestateInfo(FrameNum frame, FrameState id, uint8_t state);
	void setFramestateInfoSpecific(FrameNum frame, FrameState id, uint8_t state, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player);
	uint8_t getFramestateInfo(FrameNum frame, FrameState id) const;
	uint8_t getFramestateInfoSpecific(FrameNum frame, FrameState id, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player) const;
	// Without the id, just return the whole hog
	uint8_t getFramestateInfo(FrameNum frame) const;

	void invalidateRun(FrameNum frame);
	void invalidateRunSpecific(FrameNum frame, SavestateBlockNum savestateHookNum, BranchNum branch, uint8_t player);

	void addFrame(FrameNum afterFrame);
	void addFrameHere();
	void removeFrames(FrameNum start, FrameNum end);

	std::size_t getFramesSize() const;

	~DataProcessing();

	DECLARE_EVENT_TABLE();
};