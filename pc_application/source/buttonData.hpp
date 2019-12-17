#pragma once

#include <bitset>
#include <gdkmm/pixbuf.h>
#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <gtkmm/treemodelcolumn.h>
#include <map>
#include <utility>

// Buttons enum (coencides with the index of the bit in the input struct)
// Also used to identify the button everywhere else in the program
enum Btn {
	A,
	B,
	X,
	Y,
	L,
	R,
	ZL,
	ZR,
	SL,
	SR,
	DUP,
	DDOWN,
	DLEFT,
	DRIGHT,
	PLUS,
	MINUS,
	HOME,
	CAPT,
	LS,
	RS,
};

// Controller data that will be packed into the array and will be recieved from
// the switch
struct ControllerData {
	// This controller's index
	uint8_t index;
	// Button data (stored as a bitset because it will be serialized better later)
	// 20 buttons
	// This storage can be sacrificed for the simple reason that the other data
	// already takes up so much space
	std::bitset<20> buttons;
	// Joystick values
	int16_t LS_X = 0;
	int16_t LS_Y = 0;
	int16_t RS_X = 0;
	int16_t RS_Y = 0;
	// Gyroscope and Accelerometer data (when it is implemented)
	int16_t ACCEL_X = 0;
	int16_t ACCEL_Y = 0;
	int16_t ACCEL_Z = 0;
	int16_t GYRO_1  = 0;
	int16_t GYRO_2  = 0;
	int16_t GYRO_3  = 0;
};

// Struct containing button info
struct ButtonInfo {
	Glib::ustring scriptName;
	Glib::ustring viewName;
	Glib::RefPtr<Gdk::Pixbuf> onIcon;
	Glib::RefPtr<Gdk::Pixbuf> offIcon;
	guint toggleKeybind;
};

Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>* getNewColumn() {
	return new Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>();
}

Glib::RefPtr<Gdk::Pixbuf> getNewIcon(std::string name) {
	// https://stackoverflow.com/questions/5894344/gtkmm-how-to-put-a-pixbuf-in-a-treeview
	return Gdk::Pixbuf::create_from_file("/usr/share/icons/gnome/22x22/apps/" + name + ".png");
}

ButtonInfo* gBI(Glib::ustring scriptName, Glib::ustring viewName, Glib::RefPtr<Gdk::Pixbuf> viewIcon, Glib::RefPtr<Gdk::Pixbuf> offIcon, uint8_t toggleKeybind) {
	ButtonInfo* thisButtonInfo    = new ButtonInfo();
	thisButtonInfo->scriptName    = scriptName;
	thisButtonInfo->viewName      = viewName;
	thisButtonInfo->onIcon        = viewIcon;
	thisButtonInfo->offIcon       = offIcon;
	thisButtonInfo->toggleKeybind = toggleKeybind;
	return thisButtonInfo;
}

// Array including all button information
// Index is button Id, returns Button script name, Button print name, Button
// listview object, Button printing pixbuf when on and when off This info is
// contained in a struct
// TODO finish these
// https://gitlab.gnome.org/GNOME/gtk/blob/master/gdk/gdkkeysyms.h
std::map<Btn, ButtonInfo*> buttonMapping {
	{ Btn::A, gBI("KEY_A", "A Button", getNewIcon(""), getNewIcon(""), GDK_KEY_a) },
	{ Btn::B, gBI("KEY_B", "B Button", getNewIcon(""), getNewIcon(""), GDK_KEY_b) },
	{ Btn::X, gBI("KEY_X", "X Button", getNewIcon(""), getNewIcon(""), GDK_KEY_x) },
	{ Btn::Y, gBI("KEY_Y", "Y Button", getNewIcon(""), getNewIcon(""), GDK_KEY_y) },
	{ Btn::L, gBI("KEY_L", "L Button", getNewIcon(""), getNewIcon(""), GDK_KEY_l) },
	{ Btn::R, gBI("KEY_R", "R Button", getNewIcon(""), getNewIcon(""), GDK_KEY_r) },
	{ Btn::ZL, gBI("KEY_ZL", "ZL Button", getNewIcon(""), getNewIcon(""), GDK_KEY_o) },
	{ Btn::ZR, gBI("KEY_ZR", "ZR Button", getNewIcon(""), getNewIcon(""), GDK_KEY_p) },
	{ Btn::SL, gBI("KEY_SL", "SL Button", getNewIcon(""), getNewIcon(""), GDK_KEY_q) },
	{ Btn::SR, gBI("KEY_SR", "SR Button", getNewIcon(""), getNewIcon(""), GDK_KEY_e) },
	{ Btn::DUP, gBI("KEY_DUP", "Up Dpad", getNewIcon(""), getNewIcon(""), GDK_KEY_i) },
	{ Btn::DDOWN, gBI("KEY_DDOWN", "Down Dpad", getNewIcon(""), getNewIcon(""), GDK_KEY_k) },
	{ Btn::DLEFT, gBI("KEY_DLEFT", "Left Dpad", getNewIcon(""), getNewIcon(""), GDK_KEY_j) },
	{ Btn::DRIGHT, gBI("KEY_DRIGHT", "Right Dpad", getNewIcon(""), getNewIcon(""), GDK_KEY_l) },
	{ Btn::PLUS, gBI("KEY_PLUS", "Plus Button", getNewIcon(""), getNewIcon(""), GDK_KEY_equal) }, // Where the plus key is
	{ Btn::MINUS, gBI("KEY_MINUS", "Minus Button", getNewIcon(""), getNewIcon(""), GDK_KEY_minus) },
	{ Btn::HOME, gBI("KEY_HOME", "Home Button", getNewIcon(""), getNewIcon(""), GDK_KEY_h) },
	{ Btn::CAPT, gBI("KEY_CAPT", "Capture Button", getNewIcon(""), getNewIcon(""), GDK_KEY_g) },
	{ Btn::LS, gBI("KEY_LS", "Left Stick", getNewIcon(""), getNewIcon(""), GDK_KEY_t) },
	{ Btn::RS, gBI("KEY_RS", "Right Stick", getNewIcon(""), getNewIcon(""), GDK_KEY_y) },
};