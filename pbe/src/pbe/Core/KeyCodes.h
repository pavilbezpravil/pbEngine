#pragma once

namespace pbe
{
	typedef enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348,
		Max = Menu
	} Key;

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}
}

// From glfw3.h
#define HZ_KEY_SPACE           ::pbe::Key::Space
#define HZ_KEY_APOSTROPHE      ::pbe::Key::Apostrophe    /* ' */
#define HZ_KEY_COMMA           ::pbe::Key::Comma         /* , */
#define HZ_KEY_MINUS           ::pbe::Key::Minus         /* - */
#define HZ_KEY_PERIOD          ::pbe::Key::Period        /* . */
#define HZ_KEY_SLASH           ::pbe::Key::Slash         /* / */
#define HZ_KEY_0               ::pbe::Key::D0
#define HZ_KEY_1               ::pbe::Key::D1
#define HZ_KEY_2               ::pbe::Key::D2
#define HZ_KEY_3               ::pbe::Key::D3
#define HZ_KEY_4               ::pbe::Key::D4
#define HZ_KEY_5               ::pbe::Key::D5
#define HZ_KEY_6               ::pbe::Key::D6
#define HZ_KEY_7               ::pbe::Key::D7
#define HZ_KEY_8               ::pbe::Key::D8
#define HZ_KEY_9               ::pbe::Key::D9
#define HZ_KEY_SEMICOLON       ::pbe::Key::Semicolon     /* ; */
#define HZ_KEY_EQUAL           ::pbe::Key::Equal         /* = */
#define HZ_KEY_A               ::pbe::Key::A
#define HZ_KEY_B               ::pbe::Key::B
#define HZ_KEY_C               ::pbe::Key::C
#define HZ_KEY_D               ::pbe::Key::D
#define HZ_KEY_E               ::pbe::Key::E
#define HZ_KEY_F               ::pbe::Key::F
#define HZ_KEY_G               ::pbe::Key::G
#define HZ_KEY_H               ::pbe::Key::H
#define HZ_KEY_I               ::pbe::Key::I
#define HZ_KEY_J               ::pbe::Key::J
#define HZ_KEY_K               ::pbe::Key::K
#define HZ_KEY_L               ::pbe::Key::L
#define HZ_KEY_M               ::pbe::Key::M
#define HZ_KEY_N               ::pbe::Key::N
#define HZ_KEY_O               ::pbe::Key::O
#define HZ_KEY_P               ::pbe::Key::P
#define HZ_KEY_Q               ::pbe::Key::Q
#define HZ_KEY_R               ::pbe::Key::R
#define HZ_KEY_S               ::pbe::Key::S
#define HZ_KEY_T               ::pbe::Key::T
#define HZ_KEY_U               ::pbe::Key::U
#define HZ_KEY_V               ::pbe::Key::V
#define HZ_KEY_W               ::pbe::Key::W
#define HZ_KEY_X               ::pbe::Key::X
#define HZ_KEY_Y               ::pbe::Key::Y
#define HZ_KEY_Z               ::pbe::Key::Z
#define HZ_KEY_LEFT_BRACKET    ::pbe::Key::LeftBracket   /* [ */
#define HZ_KEY_BACKSLASH       ::pbe::Key::Backslash     /* \ */
#define HZ_KEY_RIGHT_BRACKET   ::pbe::Key::RightBracket  /* ] */
#define HZ_KEY_GRAVE_ACCENT    ::pbe::Key::GraveAccent   /* ` */
#define HZ_KEY_WORLD_1         ::pbe::Key::World1        /* non-US #1 */
#define HZ_KEY_WORLD_2         ::pbe::Key::World2        /* non-US #2 */

/* Function keys */
#define HZ_KEY_ESCAPE          ::pbe::Key::Escape
#define HZ_KEY_ENTER           ::pbe::Key::Enter
#define HZ_KEY_TAB             ::pbe::Key::Tab
#define HZ_KEY_BACKSPACE       ::pbe::Key::Backspace
#define HZ_KEY_INSERT          ::pbe::Key::Insert
#define HZ_KEY_DELETE          ::pbe::Key::Delete
#define HZ_KEY_RIGHT           ::pbe::Key::Right
#define HZ_KEY_LEFT            ::pbe::Key::Left
#define HZ_KEY_DOWN            ::pbe::Key::Down
#define HZ_KEY_UP              ::pbe::Key::Up
#define HZ_KEY_PAGE_UP         ::pbe::Key::PageUp
#define HZ_KEY_PAGE_DOWN       ::pbe::Key::PageDown
#define HZ_KEY_HOME            ::pbe::Key::Home
#define HZ_KEY_END             ::pbe::Key::End
#define HZ_KEY_CAPS_LOCK       ::pbe::Key::CapsLock
#define HZ_KEY_SCROLL_LOCK     ::pbe::Key::ScrollLock
#define HZ_KEY_NUM_LOCK        ::pbe::Key::NumLock
#define HZ_KEY_PRINT_SCREEN    ::pbe::Key::PrintScreen
#define HZ_KEY_PAUSE           ::pbe::Key::Pause
#define HZ_KEY_F1              ::pbe::Key::F1
#define HZ_KEY_F2              ::pbe::Key::F2
#define HZ_KEY_F3              ::pbe::Key::F3
#define HZ_KEY_F4              ::pbe::Key::F4
#define HZ_KEY_F5              ::pbe::Key::F5
#define HZ_KEY_F6              ::pbe::Key::F6
#define HZ_KEY_F7              ::pbe::Key::F7
#define HZ_KEY_F8              ::pbe::Key::F8
#define HZ_KEY_F9              ::pbe::Key::F9
#define HZ_KEY_F10             ::pbe::Key::F10
#define HZ_KEY_F11             ::pbe::Key::F11
#define HZ_KEY_F12             ::pbe::Key::F12
#define HZ_KEY_F13             ::pbe::Key::F13
#define HZ_KEY_F14             ::pbe::Key::F14
#define HZ_KEY_F15             ::pbe::Key::F15
#define HZ_KEY_F16             ::pbe::Key::F16
#define HZ_KEY_F17             ::pbe::Key::F17
#define HZ_KEY_F18             ::pbe::Key::F18
#define HZ_KEY_F19             ::pbe::Key::F19
#define HZ_KEY_F20             ::pbe::Key::F20
#define HZ_KEY_F21             ::pbe::Key::F21
#define HZ_KEY_F22             ::pbe::Key::F22
#define HZ_KEY_F23             ::pbe::Key::F23
#define HZ_KEY_F24             ::pbe::Key::F24
#define HZ_KEY_F25             ::pbe::Key::F25

/* Keypad */
#define HZ_KEY_KP_0            ::pbe::Key::KP0
#define HZ_KEY_KP_1            ::pbe::Key::KP1
#define HZ_KEY_KP_2            ::pbe::Key::KP2
#define HZ_KEY_KP_3            ::pbe::Key::KP3
#define HZ_KEY_KP_4            ::pbe::Key::KP4
#define HZ_KEY_KP_5            ::pbe::Key::KP5
#define HZ_KEY_KP_6            ::pbe::Key::KP6
#define HZ_KEY_KP_7            ::pbe::Key::KP7
#define HZ_KEY_KP_8            ::pbe::Key::KP8
#define HZ_KEY_KP_9            ::pbe::Key::KP9
#define HZ_KEY_KP_DECIMAL      ::pbe::Key::KPDecimal
#define HZ_KEY_KP_DIVIDE       ::pbe::Key::KPDivide
#define HZ_KEY_KP_MULTIPLY     ::pbe::Key::KPMultiply
#define HZ_KEY_KP_SUBTRACT     ::pbe::Key::KPSubtract
#define HZ_KEY_KP_ADD          ::pbe::Key::KPAdd
#define HZ_KEY_KP_ENTER        ::pbe::Key::KPEnter
#define HZ_KEY_KP_EQUAL        ::pbe::Key::KPEqual

#define HZ_KEY_LEFT_SHIFT      ::pbe::Key::LeftShift
#define HZ_KEY_LEFT_CONTROL    ::pbe::Key::LeftControl
#define HZ_KEY_LEFT_ALT        ::pbe::Key::LeftAlt
#define HZ_KEY_LEFT_SUPER      ::pbe::Key::LeftSuper
#define HZ_KEY_RIGHT_SHIFT     ::pbe::Key::RightShift
#define HZ_KEY_RIGHT_CONTROL   ::pbe::Key::RightControl
#define HZ_KEY_RIGHT_ALT       ::pbe::Key::RightAlt
#define HZ_KEY_RIGHT_SUPER     ::pbe::Key::RightSuper
#define HZ_KEY_MENU            ::pbe::Key::Menu

// Mouse (TODO: move into separate file probably)
#define HZ_MOUSE_BUTTON_LEFT    0
#define HZ_MOUSE_BUTTON_RIGHT   1
#define HZ_MOUSE_BUTTON_MIDDLE  2
