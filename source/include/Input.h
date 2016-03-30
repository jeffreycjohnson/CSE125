#ifndef INCLUDE_INPUT_H
#define INCLUDE_INPUT_H

#include "ForwardDecs.h"
#include <unordered_map>
#include <glfw3.h>

enum AxisType
{
	X, Y, SCROLL_X, SCROLL_Y
};

enum Joystick // Wraps GLFW's Joystick enum, but adds JOYSTICK_ALL as an option
{
	JOYSTICK_1, JOYSTICK_2, JOYSTICK_3, JOYSTICK_4, JOYSTICK_5, JOYSTICK_6, JOYSTICK_7, JOYSTICK_8,
	JOYSTICK_9, JOYSTICK_10, JOYSTICK_11, JOYSTICK_12, JOYSTICK_13, JOYSTICK_14, JOYSTICK_15, JOYSTICK_16,
	JOYSTICK_ALL
};

enum InputType
{
	MOUSE, KEYBOARD, JOYSTICK
};

enum InputState
{
	UNDEFINED = -1, IDLE, BUTTON_UP, BUTTON_DOWN, PRESSED
};

struct GLFWinput // Used to determine which input type it is and what GLFW enum value it maps to
{
	InputType type;
	int id;
};

struct Button
{
	InputState state;
	bool edge;
	float value;
	float startTime;
	float startValue;
};

struct InputData // User created input maps
{
	std::string name;
	std::string negativeButton;
	std::string positiveButton;
	std::string altNegativeButton;
	std::string altPositiveButton;
	float dead;
	float sensitivity;
	bool invert;
	AxisType axis;
	Joystick joystick;
};

class Input
{
private:
	static GLFWcursor* cursor;
	static GLFWwindow* window;
	static std::unordered_map<std::string, GLFWinput> inputMap; // Used to translate button names to GLFW values
	static std::unordered_map<int, Button> keyboardMap;
	static std::unordered_map<int, Button> mouseMap;
	static std::unordered_map<int, Button> joystickMap;
	static glm::vec2 scrollBuff, scrollAmount;

	static void changeState(std::unordered_map<int, Button>::iterator, int);
	static float getAxisHelper(GLFWinput, InputData);
	static InputState getButtonHelper(std::string);
	static InputState GLFWInputToState(GLFWinput);

public:
	static std::unordered_map<std::string, InputData> inputs; // Finds custom named inputs

	Input();
	~Input();

	static void init(GLFWwindow* win);
	static void update();

	static void addInput(InputData data);

	static glm::vec2 mousePosition();

	static float getAxis(std::string name);
	static bool getButtonDown(std::string name);
	static bool getButton(std::string name);
	static bool getButtonUp(std::string name);
	static bool getButtonIdle(std::string name);

	static bool getKeyDown(std::string button);
	static bool getKey(std::string button);
	static bool getKeyUp(std::string button);
	static bool getKeyIdle(std::string button);

	static bool getMouseDown(std::string button);
	static bool getMouse(std::string button);
	static bool getMouseUp(std::string button);
	static bool getMouseIdle(std::string button);

	// Cursor functions
	static bool setCursor(std::string, int, int, int = 0, int = 0);
	static bool setCursor(const GLFWimage*, int = 0, int = 0);
	static bool setCursor(int standardCursor);
	static void hideCursor();
	static void showCursor();

	static void scroll_callback(GLFWwindow*, double, double);
};

#endif