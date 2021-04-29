#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <iostream>
#include <windows.h>
#include <WinUser.h>
#include <SFML/Graphics.hpp>
//#include <SFML/System.hpp>
//#include <SFML/Window.hpp>
#include <thread>
#include <math.h>
#include <vector>
#include <array>
#include <fstream>
#include <string.h>
#include <string>
#include <locale>

static const int fieldSize = 64;			// size of game field
static bool field[fieldSize][fieldSize];	// field array of bool
static bool isGameLife = false;				// boolean - is the game running
static bool isHelpActive = true;			// boolean - is need to show help text
static const float cellSize = 16;			// size of one cell in pixels
static int simSpeed = 60;					// delay in one turn function
static sf::RenderWindow windowMain;			// main window 
static sf::RenderWindow controlWindow;		// graph window
static sf::VertexArray quads;				// array of all vertices
static sf::VertexBuffer buffer(sf::Quads);	// buffer for vertices
static std::string loadFileName;			// text in loadFileTextBox

struct AbleCharsStruct
{
	int ableChars[128];
};	// struct to transfer able chars

// class of text to draw in windows
class Text : public sf::Drawable, public sf::Transformable
{
	sf::Font font;	// font of text
	sf::Text text;	// standart text in sfml
public:

	// override '=' operator:
	Text& operator=(Text other) {
		font = other.font;	// equal font
		text = other.text;	// equal text
		text.setFont(font);
		return *this;
	}

	// constructor:
	Text(std::string string, int fontSize) {
		if (!font.loadFromFile("arial.ttf"))		// Nothing wrong with loading the font
			std::exit(-1);							// exit programm
		text = sf::Text(string, font, fontSize);	// setting default string to draw
	}
	// empty constructor:
	Text() { }

	// virtual draw function (draw in window)
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		states.transform *= getTransform();
		target.draw(text, states);
	}
};
// class of button
class Button : public sf::Drawable, public sf::Transformable
{

private:

	sf::VertexArray vertices;	// button rectangle vertices
	Text text;				// text field on button

public:

	sf::Color notPressedColor;		// button color when not pressed
	sf::Color PressedColor;			// button color when pressed
	sf::Vector2u firstPoint;		// left above poitn
	sf::Vector2u secondPoint;		// right down point
	int(*ptrFunction)(bool, bool, bool) = NULL; // pointer to function that calls when button pressed
	bool isPressedPush = false;		// is button pressed
	bool isPressedToggle = false;	// toggle by pressing button
	bool isTickPush = false;		// one tick on pressed button
	std::string buttonText;			// string on button

	// function that calls whith creating an exemplar
	bool load(sf::Vector2u m_firstPoint, sf::Vector2u m_secondPoint, sf::Color m_notPressedColor, sf::Color m_PressedColor, std::string m_buttonText, int fontSize, sf::Vector2u textPosition, int(*ptrFunc)(bool, bool, bool))
	{
		// equating values
		notPressedColor = m_notPressedColor;
		PressedColor = m_PressedColor;
		ptrFunction = *ptrFunc;
		firstPoint = m_firstPoint;
		secondPoint = m_secondPoint;
		buttonText = m_buttonText;
		//----------------

		// drawing button rectangle by vertices
		vertices.setPrimitiveType(sf::PrimitiveType::Quads); // set rectangle type to array
		vertices.resize(4);	// size each rectangle to 4, because of 4 vertices

		// settig position to each vertex
		vertices[0].position = sf::Vector2f(firstPoint.x, firstPoint.y);
		vertices[1].position = sf::Vector2f(firstPoint.x, secondPoint.y);
		vertices[2].position = sf::Vector2f(secondPoint.x, secondPoint.y);
		vertices[3].position = sf::Vector2f(secondPoint.x, firstPoint.y);

		// setting color for each vertex
		vertices[0].color = m_notPressedColor;
		vertices[1].color = m_notPressedColor;
		vertices[2].color = m_notPressedColor;
		vertices[3].color = m_notPressedColor;
		//--------------------

		// setting text on top of button
		text = Text(buttonText, fontSize);
		text.setPosition(sf::Vector2f(textPosition.x, textPosition.y)); // setting sf::Text position to middle of button

		return true;
	}

	// procces current event
	void eventProcces(sf::Event m_event)
	{
		isTickPush = false;
		// check if pressed on button
		if (m_event.type == sf::Event::MouseButtonPressed && m_event.mouseButton.x >= firstPoint.x && m_event.mouseButton.x <= secondPoint.x && m_event.mouseButton.y >= firstPoint.y && m_event.mouseButton.y <= secondPoint.y)
		{
			isPressedPush = true;
			isTickPush = true;
			isPressedToggle = !isPressedToggle;

			// change button color
			vertices[0].color = PressedColor;
			vertices[1].color = PressedColor;
			vertices[2].color = PressedColor;
			vertices[3].color = PressedColor;
			//--------------------
		}

		// check if button released
		if (m_event.type == sf::Event::MouseButtonReleased)
		{
			isPressedPush = false;

			// change button color
			vertices[0].color = notPressedColor;
			vertices[1].color = notPressedColor;
			vertices[2].color = notPressedColor;
			vertices[3].color = notPressedColor;
			//--------------------
		}
		ptrFunction(isPressedPush, isPressedToggle, isTickPush); // call custom function for this button
	}

	// draw in window
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();	// transforming
		target.draw(vertices, states);		// drawing button rectangle
		target.draw(text);					// drawing text
	}
};
// class of textBox
class TextBox : public sf::Drawable, sf::Transformable
{
private:
	sf::VertexArray vertices;	// field of text box
	Text text;					// text of text box
public:
	sf::Color activeColor;		// color when active
	sf::Color noActiveColor;	// color when not active
	std::string stringText;		// string of text in box
	sf::Vector2f firstPoint;	// first position point 
	sf::Vector2f secondPoint;	// second position point
	AbleCharsStruct ableChars;	// struct to set able chars to write
	int(*ptrTextBoxEndEvent)(std::string newText) = NULL;	// pointer to user function when editing end
	int textSize = 16;			// standart text size
	bool isEverPressed = false;	// bollean is ever button pressed

	bool isActive = false;		// is text box active now

	// constructor:
	bool load(sf::Vector2f m_firstPoint,sf::Vector2f m_secondPoint, sf::Color m_activeColor, sf::Color m_noActiveColor, std::string startText, int m_textSize, sf::Vector2f textOffset)
	{
		//equalating variables:
		activeColor = m_activeColor;
		noActiveColor = m_noActiveColor;
		stringText = startText;
		firstPoint = m_firstPoint;
		secondPoint = m_secondPoint;
		textSize = m_textSize;
		//---------------------

		//drawing rectangle:
		vertices.setPrimitiveType(sf::PrimitiveType::Quads);
		vertices.resize(4);

		// setting vertices to position
		vertices[0].position = firstPoint;
		vertices[1].position = sf::Vector2f(firstPoint.x, secondPoint.y);
		vertices[2].position = secondPoint;
		vertices[3].position = sf::Vector2f(secondPoint.x, firstPoint.y);

		//setting color to vertices:
		vertices[0].color = noActiveColor;
		vertices[1].color = noActiveColor;
		vertices[2].color = noActiveColor;
		vertices[3].color = noActiveColor;

		// setting text:
		text = Text(stringText, textSize);
		text.setPosition(((firstPoint.x + secondPoint.x) / 2) + textOffset.x, ((firstPoint.y + secondPoint.y) / 2) + textOffset.y); // text position

		return true;
	}

	void eventProces(sf::Event m_event)
	{
		
		// When pressed on the text → active this text box
		if (m_event.type == sf::Event::MouseButtonPressed && m_event.mouseButton.x >= firstPoint.x && m_event.mouseButton.y >= firstPoint.y && m_event.mouseButton.x <= secondPoint.x && m_event.mouseButton.y <= secondPoint.y)
		{
			// if pressed firstly - clear text box
			if (isEverPressed == false)
			{
				isEverPressed = true;
				stringText = "";
				text = Text(stringText, textSize);
			}
			isActive = true;

			// set active color
			vertices[0].color = activeColor;
			vertices[1].color = activeColor;
			vertices[2].color = activeColor;
			vertices[3].color = activeColor;
		}
		if (isActive && m_event.type == sf::Event::TextEntered)		// When active and text keys pressed → write it to text box
		{
			if (m_event.text.unicode == '\b' && stringText != "") // if text is not empty, so we can remove last symbol
			{
				stringText.erase(stringText.size() - 1, 1); // remove last symbol
			}
			if (m_event.text.unicode != '\b' && stringText.size() < 16 && ableChars.ableChars[0] == NULL) // if text less than 16, so we can add new symbol (if all chars able)
			{
				stringText += static_cast<char>(m_event.text.unicode); // add new symbol
			}
			if (m_event.text.unicode != '\b' && stringText.size() < 16 && ableChars.ableChars[0] != NULL) // if text less than 16, so we can add new symbol (if NOT all chars able)
			{
				int i = 0;
				while (ableChars.ableChars[i] != NULL) // itarate all able symbols 
				{
					if(ableChars.ableChars[i] == static_cast<char>(m_event.text.unicode))	// check if writen symbol able
						stringText += static_cast<char>(m_event.text.unicode);				// add this symble
					i++;
				}
			}
			text = Text(stringText, textSize);	// add text to draw
		}

		if (isActive && m_event.type == sf::Event::KeyPressed && m_event.key.code == sf::Keyboard::Enter) // when 'Enter' pressed → unActive this shit
		{
			isActive = false;

			// set no active color:
			vertices[0].color = noActiveColor;
			vertices[1].color = noActiveColor;
			vertices[2].color = noActiveColor;
			vertices[3].color = noActiveColor;

			if(stringText != "" && ptrTextBoxEndEvent != NULL)	// if we can call user function
				ptrTextBoxEndEvent(stringText);					// call user function
		}
		// when pressed somewhere NOT on the text box → unActive this shit
		if (m_event.type == sf::Event::MouseButtonPressed && (m_event.mouseButton.x < firstPoint.x || m_event.mouseButton.y < firstPoint.y || m_event.mouseButton.x > secondPoint.x || m_event.mouseButton.y > secondPoint.y))
		{
			isActive = false;

			// set no active color
			vertices[0].color = noActiveColor;
			vertices[1].color = noActiveColor;
			vertices[2].color = noActiveColor;
			vertices[3].color = noActiveColor;

			//if (stringText != "" && ptrTextBoxEndEvent != NULL)
				//ptrTextBoxEndEvent(stringText);
		}
	}

	// virtual draw function
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();	// transforming
		target.draw(vertices, states);		// drawing button rectangle
		target.draw(text);					// drawing text
	}
};
// class of indicator to show bool value
class ColorIndicator : public sf::Drawable, sf::Transformable
{
private:

	sf::VertexArray vertices;	// color indicator vertices array

public:

	sf::Vector2f firstPoint;	// first position point
	sf::Vector2f secondPoint;	// second position point
	bool indicatorState = false;

	// constructor 
	bool load(sf::Vector2f m_firstPoint, sf::Vector2f m_secondPoint, bool startState)
	{
		//equalating variables:
		firstPoint = m_firstPoint;
		secondPoint = m_secondPoint;
		indicatorState = startState;
		//-------------------------

		vertices.setPrimitiveType(sf::Quads);	// setting rectangels to draw
		vertices.resize(4);						// setting size to 4 becouse of 4 vertices in rectangle

		// setting position of each vertex
		vertices[0].position = firstPoint;
		vertices[1].position = sf::Vector2f(firstPoint.x, secondPoint.y);
		vertices[2].position = secondPoint;
		vertices[3].position = sf::Vector2f(secondPoint.x, firstPoint.y);

		// reading current state of indicator
		if (indicatorState)
		{
			// if indicator is true → draw green
			vertices[0].color = sf::Color(20, 140, 20);
			vertices[1].color = sf::Color(20, 140, 20);
			vertices[2].color = sf::Color(20, 140, 20);
			vertices[3].color = sf::Color(20, 140, 20);
		}
		else
		{
			// if indicator is false → draw red
			vertices[0].color = sf::Color(140, 20, 20);
			vertices[1].color = sf::Color(140, 20, 20);
			vertices[2].color = sf::Color(140, 20, 20);
			vertices[3].color = sf::Color(140, 20, 20);
		}
		return 0;
	}

	// function to update indicator drawing state
	void update(bool newState)
	{
		indicatorState = newState; // getting new state of indicator
		if (indicatorState)
		{	
			// if state is true → draw green
			vertices[0].color = sf::Color(20, 140, 20);
			vertices[1].color = sf::Color(20, 140, 20);
			vertices[2].color = sf::Color(20, 140, 20);
			vertices[3].color = sf::Color(20, 140, 20);
		}
		else
		{
			// if state is false → draw red
			vertices[0].color = sf::Color(140, 20, 20);
			vertices[1].color = sf::Color(140, 20, 20);
			vertices[2].color = sf::Color(140, 20, 20);
			vertices[3].color = sf::Color(140, 20, 20);
		}
	}

	// virtual draw
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();	// transforming
		target.draw(vertices, states);		// drawing button rectangle
	}
};

// function to clear game field
void clearField()
{
	for (int i = 0; i < fieldSize; i++)
		for (int j = 0; j < fieldSize; j++)	// iterate each cell
			field[i][j] = false;			// satting cell to 'false'/DEAD cell
}

// method of 'Life button start'
int lifeButtonPressedEvent(bool pushIn, bool toggleIn, bool tickPush)
{
	if (tickPush)	// one tick if
		isGameLife = !isGameLife;	// change game state
	return 0;
}
// method of 'Load organism button'
int LoadButtonPressedEvent(bool pushIn, bool toggleIn, bool tickPush)
{
	if (tickPush && !isGameLife && loadFileName != "")
	{
		std::ifstream organismFile;					// file stream
		organismFile.open(loadFileName + ".txt");	// open file with organism
		std::string organismLineBuffer;				// buffer for each line of loading organism
		int j = 0;	// counter
		if (organismFile.is_open())	 // if file opened properly
		{
			while (std::getline(organismFile, organismLineBuffer))	// iterate each line in file
			{
				if(organismLineBuffer.size() <= fieldSize)	// check: is loading file fit to our field
					for (int i = 0; i < fieldSize; i++)		// iterate each symbol in line in file
					{
						if (organismLineBuffer[i] == '0')	// check in buffer
							field[i][j] = false;			// set right state
						if (organismLineBuffer[i] == '1')	// check in buffer
							field[i][j] = true;				// set right state
					}
				j++; // counter ++
			}
		}

		//to save new struct :
		/*
		else
		{
			for (int i = 0; i < fieldSize; i++)
			{
				for (int j = 0; j < fieldSize; j++)
				{
					std::cout << field[j][i];
				}
				std::cout << "\n";
			}
		}
		*/
	}

	return 0;
}
// method of 'show help button'
int helpButtonPressedEvent(bool pushIn, bool toggleIn, bool tickPush)
{
	if (tickPush) // one tick if
	{
		isHelpActive = !isHelpActive;	// set help value to opposite
		isGameLife = false;				// stoping game 
	}
	return 0;
}
// method of 'sim speed textBox end editing'
int simSpeedTextBoxEndEvent(std::string newText)
{
	simSpeed = std::stoi(newText);	// setting simSpeed to new
	return 0;
}
// method of 'clearing field by button'
int clearButtonEvent(bool pushIn, bool toggleIn, bool tickPush)
{
	if (tickPush)
		clearField();
	return 0;
}

// method of game turn
void turn()
{
	bool resField[fieldSize][fieldSize]; // new field after turn calculation
	while (true) // cycle for turns
	{
		Sleep(simSpeed);	// delay to control simSpeed
		if (isGameLife)		// check if game is started
		{
			// setting all cells of result field to 'false'
			for (int i = 0; i < fieldSize; i++)
				for (int j = 0; j < fieldSize; j++)
					resField[i][j] = false;

			int nearCelsCount = 0;					// counter of cells nearby
			// iterate each cell
			for (int i = 0; i < fieldSize; i++)
			{
				for (int j = 0; j < fieldSize; j++)
				{
					nearCelsCount = 0;								// set counter of cells nearby to zero

					if (i == 0 && j != 0 && j != fieldSize - 1)
					{
						// if current cell is there:
						/*
						. i i i .
						. . . . .
						. . . . .
						. . . . .
						. . . . .
						*/
						// check specific cells near:
						if (field[i][j - 1])
							nearCelsCount++;
						if (field[i][j + 1])
							nearCelsCount++;
						if (field[i + fieldSize - 1][j + 1])
							nearCelsCount++;
						if (field[i + fieldSize - 1][j - 1])
							nearCelsCount++;
						if (field[i + fieldSize - 1][j])
							nearCelsCount++;
						if (field[i + 1][j + 1])
							nearCelsCount++;
						if (field[i + 1][j - 1])
							nearCelsCount++;
						if (field[i + 1][j])
							nearCelsCount++;
					}
					if (i == fieldSize - 1 && j != 0 && j != fieldSize - 1)
					{
						// if current cell is there:
						/*
						. . . . .
						. . . . .
						. . . . .
						. . . . .
						. i i i .
						*/
						// check specific cells near:
						if (field[i][j - 1])
							nearCelsCount++;
						if (field[i][j + 1])
							nearCelsCount++;
						if (field[i - 1][j + 1])
							nearCelsCount++;
						if (field[i - 1][j - 1])
							nearCelsCount++;
						if (field[i - 1][j])
							nearCelsCount++;
						if (field[0][j + 1])
							nearCelsCount++;
						if (field[0][j - 1])
							nearCelsCount++;
						if (field[0][j])
							nearCelsCount++;
					}
					if (j == 0 && i != 0 && i != fieldSize - 1)
					{
						// if current cell is there:
						/*
						. . . . .
						j . . . .
						j . . . .
						j . . . .
						. . . . .
						*/
						// check specific cells near:
						if (field[i][j + fieldSize - 1])
							nearCelsCount++;
						if (field[i][j + 1])
							nearCelsCount++;
						if (field[i - 1][j + 1])
							nearCelsCount++;
						if (field[i - 1][j + fieldSize - 1])
							nearCelsCount++;
						if (field[i - 1][j])
							nearCelsCount++;
						if (field[i + 1][j + 1])
							nearCelsCount++;
						if (field[i + 1][j + fieldSize - 1])
							nearCelsCount++;
						if (field[i + 1][j])
							nearCelsCount++;
					}

					if (j == fieldSize - 1 && i != 0 && i != fieldSize - 1)
					{
						// if current cell is there:
						/*
						. . . . .
						. . . . j
						. . . . j
						. . . . j
						. . . . .
						*/
						// check specific cells near:
						if (field[i][j - 1])
							nearCelsCount++;
						if (field[i][0])
							nearCelsCount++;
						if (field[i - 1][0])
							nearCelsCount++;
						if (field[i - 1][j - 1])
							nearCelsCount++;
						if (field[i - 1][j])
							nearCelsCount++;
						if (field[i + 1][0])
							nearCelsCount++;
						if (field[i + 1][j - 1])
							nearCelsCount++;
						if (field[i + 1][j])
							nearCelsCount++;
					}

					if (i == 0 && j == 0)
					{
						// if current cell is there:
						/*
						i . . . 1
						. . . . 1
						. . . . .
						. . . . .
						1 1 . . 1
						*/
						// check specific cells near:
						if (field[i][j + 1])
							nearCelsCount++;
						if (field[i + 1][j + 1])
							nearCelsCount++;
						if (field[i + 1][j])
							nearCelsCount++;
						if (field[fieldSize - 1][0])
							nearCelsCount++;
						if (field[0][fieldSize - 1])
							nearCelsCount++;
						if (field[fieldSize - 1][fieldSize - 1])
							nearCelsCount++;
						if (field[1][fieldSize - 1])
							nearCelsCount++;
						if (field[fieldSize - 1][1])
							nearCelsCount++;
					}

					if (i == 0 && j == fieldSize - 1)
					{
						// if current cell is there:
						/*
						1 . . . i
						1 . . . .
						. . . . .
						. . . . .
						1 . . 1 1
						*/
						// check specific cells near:
						if (field[i][j - 1])
							nearCelsCount++;
						if (field[i + 1][j - 1])
							nearCelsCount++;
						if (field[i + 1][j])
							nearCelsCount++;
						if (field[0][0])
							nearCelsCount++;
						if (field[fieldSize - 1][0])
							nearCelsCount++;
						if (field[fieldSize - 1][fieldSize - 1])
							nearCelsCount++;
						if (field[1][0])
							nearCelsCount++;
						if (field[fieldSize - 1][fieldSize - 2])
							nearCelsCount++;
					}

					if (i == fieldSize - 1 && j == 0)
					{
						// if current cell is there:
						/*
						1 1 . . 1
						. . . . .
						. . . . .
						. . . . 1
						i . . . 1
						*/
						// check specific cells near:
						if (field[i][j + 1])
							nearCelsCount++;
						if (field[i - 1][j + 1])
							nearCelsCount++;
						if (field[i - 1][j])
							nearCelsCount++;
						if (field[0][0])
							nearCelsCount++;
						if (field[0][fieldSize - 1])
							nearCelsCount++;
						if (field[fieldSize - 1][fieldSize - 1])
							nearCelsCount++;
						if (field[0][1])
							nearCelsCount++;
						if (field[fieldSize - 2][fieldSize - 1])
							nearCelsCount++;
					}

					if (i == fieldSize - 1 && j == fieldSize - 1)
					{
						// if current cell is there:
						/*
						1 . . 1 1
						. . . . .
						. . . . .
						1 . . . .
						1 . . . i
						*/
						// check specific cells near:
						if (field[i][j - 1])
							nearCelsCount++;
						if (field[i - 1][j - 1])
							nearCelsCount++;
						if (field[i - 1][j])
							nearCelsCount++;
						if (field[0][0])
							nearCelsCount++;
						if (field[0][fieldSize - 1])
							nearCelsCount++;
						if (field[fieldSize - 1][0])
							nearCelsCount++;
						if (field[fieldSize - 2][0])
							nearCelsCount++;
						if (field[0][fieldSize - 2])
							nearCelsCount++;
					}

					if (i != 0 && i != fieldSize - 1 && j != 0 && j != fieldSize - 1)
					{
						// if current cell is there:
						/*
						. . . . .
						. i i i .
						. i i i .
						. i i i .
						. . . . .
						*/
						// check specific cells near:
						if (field[i][j - 1])
							nearCelsCount++;
						if (field[i][j + 1])
							nearCelsCount++;
						if (field[i - 1][j + 1])
							nearCelsCount++;
						if (field[i - 1][j - 1])
							nearCelsCount++;
						if (field[i - 1][j])
							nearCelsCount++;
						if (field[i + 1][j + 1])
							nearCelsCount++;
						if (field[i + 1][j - 1])
							nearCelsCount++;
						if (field[i + 1][j])
							nearCelsCount++;
					}

					if (!field[i][j]) // check if current cell is DEAD
					{
						if (nearCelsCount == 3)		// if there is enough cells to become LIFE
							resField[i][j] = true;	// now it has LIFE
					}
					else // if cell is LIFE
					{
						if (nearCelsCount == 3 || nearCelsCount == 2)	// if there is enough cells to stay LIFE
							resField[i][j] = true;						// this cell still has life
					}

					// temp (to make maze):
					//field[i][j] = resField[i][j];
				}
			}

			// temp commented (to make Life Game):
			for (int i = 0; i < fieldSize; i++)
				for (int j = 0; j < fieldSize; j++)
					field[i][j] = resField[i][j];
		}
	}
}

// function to show and draw main game field
void showGraphField(bool ptr[][fieldSize], int fps)
{
	sf::Text helpText;										// variable to containe help text
	sf::Font font; font.loadFromFile("arial.TTF");			// setting programm font
	helpText.setCharacterSize(cellSize*fieldSize / 35);		// set character size of help text
	helpText.setFont(font);									// applying font to help text
	// setting strig to help text
	helpText.setString("\t\tHELLO! this is Conway's Game of Life!\n\nto start your game you need to draw first generation of cells on gray window\nusing mouse control.\n\nTo calculate next generations you need to press \"Life Button\"\nin another window.\n\nYou can stop simulation anytime you want to draw or remove cells from field.\n\nEnjoy!");

	// setting vertecies buffer:
	buffer.create(fieldSize*fieldSize * 4);				// buffer create with count of cells multiplayed on one cell vertices
	buffer.setUsage(sf::VertexBuffer::Usage::Stream);	// setting usage as stream

	sf::ContextSettings settings;						// settings variable
	settings.antialiasingLevel = 8;						// setting antialiasing
	// create window:
	windowMain.create(sf::VideoMode(fieldSize*cellSize, fieldSize*cellSize), "Hellow world!", sf::Style::Titlebar | sf::Style::Close, settings);
	windowMain.setFramerateLimit(60);				// set frame rate as 60

	windowMain.clear(sf::Color(255, 230, 185, 0));	// clear window to color background
	sf::Event event;								// event handler
	bool setTo = false;								// variable to save state of cell when user pressed on it to draw organism
	quads.setPrimitiveType(sf::Quads);				// set vertex array type to draw rectangels
	quads.resize(fieldSize * fieldSize * 4);		// resize vertex array with all cells vertices count
	while (windowMain.isOpen())						// while window opened
	{
		while (windowMain.pollEvent(event))			// poll event
		{
			if (event.type == sf::Event::Closed)	// when user close window 
			{
				windowMain.close();	// close window
				exit(1);			// stop programm
			}

			// when user pressed on field to draw organism
			if (event.type == sf::Event::MouseButtonPressed && !isGameLife && sf::Mouse::isButtonPressed(sf::Mouse::Left) && event.type != sf::Event::GainedFocus) 
			{
				// get current state of pressed cell:
				setTo = !ptr[event.mouseButton.x / (windowMain.getSize().x / fieldSize)][event.mouseButton.y / (windowMain.getSize().y / fieldSize)];
			}

			if (event.type == sf::Event::LostFocus)	// if to avoid problems with getting drawing when focus
			{
				setTo = true;
			}

			// if user still pressing mouse button → draw wth remembered state
			if (!isGameLife && sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				sf::Vector2i pos = sf::Mouse::getPosition(windowMain);	// get mouse position
				// check if mouse in window:
				if(pos.x > 0 && pos.x < fieldSize*cellSize && pos.y > 0 && pos.y < fieldSize * cellSize)
					{
						//windowMain.requestFocus();
						ptr[pos.x / (windowMain.getSize().x / fieldSize)][pos.y / (windowMain.getSize().y / fieldSize)] = setTo;
					}
			}
			// when pressed somewhere to draw → close help
			if (event.type == sf::Event::MouseButtonPressed)
			{
				isHelpActive = false; // close help
			}
		}

		//				 -drawing cells:-
		// iterate each cell:
		for (int i = 0; i < fieldSize; i++)
			for (int j = 0; j < fieldSize; j++)
			{
				sf::Vertex* quad = &quads[(i + j * fieldSize) * 4];							// get 4 vertices of current cell

				quad[0].position = sf::Vector2f(i * cellSize, j * cellSize);				// set each vertex of cell to position:
				quad[1].position = sf::Vector2f((i + 1) * cellSize, j * cellSize);
				quad[2].position = sf::Vector2f((i + 1) * cellSize, (j + 1) * cellSize);
				quad[3].position = sf::Vector2f(i * cellSize, (j + 1) * cellSize);

				// coloring cells:
				if (field[i][j]) {
					// to life color
					quad[0].color = sf::Color(210, 110, 110);
					quad[1].color = sf::Color(210, 110, 110);
					quad[2].color = sf::Color(210, 110, 110);
					quad[3].color = sf::Color(210, 110, 110);
				}
				else
				{
					// to dead color
					quad[0].color = sf::Color(110, 110, 110);
					quad[1].color = sf::Color(110, 110, 110);
					quad[2].color = sf::Color(110, 110, 110);
					quad[3].color = sf::Color(110, 110, 110);
				}
			}
		//-

		//- buffer update
		buffer.update(&quads[0]);
		//- 

		windowMain.draw(buffer);		// draw buffer
		if (isHelpActive)				// if help needs to draw
			windowMain.draw(helpText);	// draw help text

		windowMain.display();			// display window
	}
}

// function to show and draw game control window
void showControlPanel()
{
	// creating window:
	controlWindow.create(sf::VideoMode(400, 200), "Life control panel", sf::Style::Titlebar | sf::Style::Close);	// create
	controlWindow.clear(sf::Color(255, 230, 185, 0));																// set background color
	controlWindow.setVerticalSyncEnabled(true);																		// vSync on
	controlWindow.setFramerateLimit(60);																			// set frame rate as 60

	sf::Event event;	// event handler

	// init control window parts variabls
	Button clearButton;			// clear button var
	clearButton.load(sf::Vector2u(240, 10), sf::Vector2u(300, 40), sf::Color(170, 170, 170), sf::Color(130, 130, 130), "clear", 18, sf::Vector2u(250,13), &clearButtonEvent);
	Button LifeButton;			// button to start/stop game 
	LifeButton.load(sf::Vector2u(10, 10), sf::Vector2u(150, 75), sf::Color(170, 170, 170), sf::Color(130, 130, 130), "Game\nbutton", 23, sf::Vector2u(35, 14), &lifeButtonPressedEvent);
	Button LoadButton;			// load organism from file button
	LoadButton.load(sf::Vector2u(170, 90), sf::Vector2u(320, 120), sf::Color(170, 170, 170), sf::Color(130, 130, 130), "Load organism", 16, sf::Vector2u(190, 95), &LoadButtonPressedEvent);
	Button helpButton;			// show/hide help text button
	helpButton.load(sf::Vector2u(170, 10), sf::Vector2u(230, 40), sf::Color(170, 170, 170), sf::Color(130, 130, 130), "help", 18, sf::Vector2u(183, 13), &helpButtonPressedEvent);
	TextBox loadedOrganismNameTextBox;	// text box to input name of organism which user want to load
	loadedOrganismNameTextBox.load(sf::Vector2f(170, 50), sf::Vector2f(320, 80), sf::Color(180, 180, 180), sf::Color(130, 130, 130), "load organism name", 16, sf::Vector2f(-70, -9));
	TextBox simSpeedTextBox;	// text box to change simulation speed
	simSpeedTextBox.ptrTextBoxEndEvent = &simSpeedTextBoxEndEvent;	// change sim speed when pressed 'Enter'
	simSpeedTextBox.load(sf::Vector2f(170, 130), sf::Vector2f(320, 160), sf::Color(180, 180, 180), sf::Color(130, 130, 130), "sim speed", 17, sf::Vector2f(-60, -10));
	//	get struct of able chars to write in simSpeed text box
	AbleCharsStruct intCharsAble;
	intCharsAble.ableChars[0] = '0';
	intCharsAble.ableChars[1] = '1';
	intCharsAble.ableChars[2] = '2';
	intCharsAble.ableChars[3] = '3';
	intCharsAble.ableChars[4] = '4';
	intCharsAble.ableChars[5] = '5';
	intCharsAble.ableChars[6] = '6';
	intCharsAble.ableChars[7] = '7';
	intCharsAble.ableChars[8] = '8';
	intCharsAble.ableChars[9] = '9';
	simSpeedTextBox.ableChars = intCharsAble; 	// set struct of able chars to write in simSpeed text box
	ColorIndicator lifeIndicator1;				// indicator is game life
	lifeIndicator1.load(sf::Vector2f(140, 10), sf::Vector2f(150, 75), false);

	while (controlWindow.isOpen())								// while window open
	{
		loadFileName = loadedOrganismNameTextBox.stringText;	// get name of organism which user want to load
		controlWindow.clear(sf::Color(255, 230, 185, 0));		// color background
		while (controlWindow.pollEvent(event))					// poll event
		{
			// transfer event to exemplar's methods:
			clearButton.eventProcces(event);
			LifeButton.eventProcces(event);
			LoadButton.eventProcces(event);
			helpButton.eventProcces(event);
			loadedOrganismNameTextBox.eventProces(event);
			simSpeedTextBox.eventProces(event);
			lifeIndicator1.update(isGameLife); // updating indicator

			if (event.type == sf::Event::Closed) // if user close window
			{
				controlWindow.close();	// close window
				exit(1);				// exit programm
			}
		}

		/////////////////// drawing stuff
		controlWindow.draw(clearButton);
		controlWindow.draw(LifeButton);
		controlWindow.draw(LoadButton);
		controlWindow.draw(helpButton);
		controlWindow.draw(loadedOrganismNameTextBox);
		controlWindow.draw(lifeIndicator1);
		controlWindow.draw(simSpeedTextBox);
		///////////////////

		controlWindow.display();	// display control window
	}
}

// main:
int main()
{
	// hiding console:
	//HWND hWnd = GetConsoleWindow();
	//ShowWindow(hWnd, SW_HIDE);
	//----------------

	std::thread thr3(turn);							// start game cycle in separeted thread
	std::thread thr1(showGraphField, field, 60);	// show game field in separeted thread
		
	Sleep(200);										// wait untill 'turn' and 'showGraphField' fully load to avoid crashes
	std::thread thr(showControlPanel);				// show control panel in separeted thread

	std::cin.get();									// wait pressing in console to avoid early closing
}
