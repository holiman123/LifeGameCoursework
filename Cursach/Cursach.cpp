#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <iostream>
#include <windows.h>
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

	bool load(sf::Vector2f m_firstPoint, sf::Vector2f m_secondPoint, bool startState)
	{
		firstPoint = m_firstPoint;
		secondPoint = m_secondPoint;
		indicatorState = startState;

		vertices.setPrimitiveType(sf::Quads);
		vertices.resize(4);

		vertices[0].position = firstPoint;
		vertices[1].position = sf::Vector2f(firstPoint.x, secondPoint.y);
		vertices[2].position = secondPoint;
		vertices[3].position = sf::Vector2f(secondPoint.x, firstPoint.y);

		if (indicatorState)
		{
			vertices[0].color = sf::Color(20, 140, 20);
			vertices[1].color = sf::Color(20, 140, 20);
			vertices[2].color = sf::Color(20, 140, 20);
			vertices[3].color = sf::Color(20, 140, 20);
		}
		else
		{
			vertices[0].color = sf::Color(140, 20, 20);
			vertices[1].color = sf::Color(140, 20, 20);
			vertices[2].color = sf::Color(140, 20, 20);
			vertices[3].color = sf::Color(140, 20, 20);
		}
		return 0;
	}

	void update(bool newState)
	{
		indicatorState = newState;
		if (indicatorState)
		{
			vertices[0].color = sf::Color(20, 140, 20);
			vertices[1].color = sf::Color(20, 140, 20);
			vertices[2].color = sf::Color(20, 140, 20);
			vertices[3].color = sf::Color(20, 140, 20);
		}
		else
		{
			vertices[0].color = sf::Color(140, 20, 20);
			vertices[1].color = sf::Color(140, 20, 20);
			vertices[2].color = sf::Color(140, 20, 20);
			vertices[3].color = sf::Color(140, 20, 20);
		}
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();	// transforming
		target.draw(vertices, states);		// drawing button rectangle
	}
};

void clearField()
{
	for (int i = 0; i < fieldSize; i++)
		for (int j = 0; j < fieldSize; j++)
			field[i][j] = false;
}

int lifeButtonStartPressedEvent(bool pushIn, bool toggleIn, bool tickPush)
{
	if (tickPush)
		isGameLife = true;

	return 0;
}
int lifeButtonStopPressedEvent(bool pushIn, bool toggleIn, bool tickPush)
{
	if (tickPush)
		isGameLife = false;
	return 0;
}
int LoadButtonPressedEvent(bool pushIn, bool toggleIn, bool tickPush)
{
	if (tickPush && !isGameLife && loadFileName != "")
	{
		std::ifstream organismFile;
		organismFile.open(loadFileName + ".txt");
		std::string organismBuffer;
		std::string organismLineBuffer;
		int j = 0;
		if (organismFile.is_open())
		{
			while (std::getline(organismFile, organismLineBuffer))
			{
				if(organismLineBuffer.size() <= fieldSize)
					for (int i = 0; i < fieldSize; i++)
					{
						if (organismLineBuffer[i] == '0')
							field[i][j] = false;
						if (organismLineBuffer[i] == '1')
							field[i][j] = true;
					}
				j++;
			}
			std::cout << organismBuffer;
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
int SaveButtonPressedEvent(bool pushIn, bool toggleIn, bool tickPush)
{

	return 0;
}
int helpButtonPressedEvent(bool pushIn, bool toggleIn, bool tickPush)
{
	if (tickPush)
	{
		isHelpActive = !isHelpActive;
		clearField();
		isGameLife = false;
	}
	return 0;
}
int simSpeedTextBoxEndEvent(std::string newText)
{
	simSpeed = std::stoi(newText);
	return 0;
}

void turn()
{
	while (true)
	{
		Sleep(simSpeed);
		if (isGameLife)
		{
			bool resField[fieldSize][fieldSize];

			for (int i = 0; i < fieldSize; i++)
				for (int j = 0; j < fieldSize; j++)
					resField[i][j] = false;

			int nearCelsCount = 0;
			for (int i = 0; i < fieldSize; i++)
			{
				for (int j = 0; j < fieldSize; j++)
				{
					nearCelsCount = 0;
					if (i == 0 && j != 0 && j != fieldSize - 1)
					{
						/*
						. i i i .
						. . . . .
						. . . . .
						. . . . .
						. . . . .
						*/
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
						/*
						. . . . .
						. . . . .
						. . . . .
						. . . . .
						. i i i .
						*/
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
						/*
						. . . . .
						j . . . .
						j . . . .
						j . . . .
						. . . . .
						*/
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
						/*
						. . . . .
						. . . . j
						. . . . j
						. . . . j
						. . . . .
						*/
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
						/*
						i . . . 1
						. . . . 1
						. . . . .
						. . . . .
						1 1 . . 1
						*/
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
						/*
						1 . . . i
						1 . . . .
						. . . . .
						. . . . .
						1 . . 1 1
						*/
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
						/*
						1 1 . . 1
						. . . . .
						. . . . .
						. . . . 1
						i . . . 1
						*/
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
						/*
						1 . . 1 1
						. . . . .
						. . . . .
						1 . . . .
						1 . . . i
						*/
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
						/*
						. . . . .
						. i i i .
						. i i i .
						. i i i .
						. . . . .
						*/
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

					if (!field[i][j]) // if DEAD
					{
						if (nearCelsCount == 3)
							resField[i][j] = true; // now it has LIFE
					}
					else // if LIFE
					{
						if (nearCelsCount == 3 || nearCelsCount == 2)
							resField[i][j] = true; // this cell still has life
					}
				}
			}
			for (int i = 0; i < fieldSize; i++)
				for (int j = 0; j < fieldSize; j++)
					field[i][j] = resField[i][j];
		}
	}
}

void showGraphField(bool ptr[][fieldSize], int fps)
{
	sf::Text helpText;
	sf::Font font; font.loadFromFile("arial.TTF");
	helpText.setCharacterSize(cellSize*fieldSize / 35);
	helpText.setFont(font);
	helpText.setString("\t\tHELLO! this is Conway's Game of Life!\n\nto start your game you need to draw first generation of cells on gray window\nusing mouse control.\n\nTo calculate next generations you need to press \"Life Button\"\nin another window.\n\nYou can stop simulation anytime you want to draw or remove cells from field.\n\nEnjoy!");

	buffer.create(fieldSize*fieldSize * 4);
	buffer.setUsage(sf::VertexBuffer::Usage::Stream);

	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	windowMain.create(sf::VideoMode(fieldSize*cellSize, fieldSize*cellSize), "Hellow world!", sf::Style::Titlebar | sf::Style::Close, settings);
	windowMain.setFramerateLimit(60);

	windowMain.clear(sf::Color(255, 230, 185, 0));
	sf::Event event;
	bool setTo = false;
	quads.setPrimitiveType(sf::Quads);
	quads.resize(fieldSize * fieldSize * 4);
	while (windowMain.isOpen())
	{
		while (windowMain.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) {
				windowMain.close();
				exit(1);
			}

			if (event.type == sf::Event::MouseButtonPressed && !isGameLife && sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				setTo = !ptr[event.mouseButton.x / (windowMain.getSize().x / fieldSize)][event.mouseButton.y / (windowMain.getSize().y / fieldSize)];
			}
			if (!isGameLife && sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				sf::Vector2i pos = sf::Mouse::getPosition(windowMain);
				if(pos.x > 0 && pos.x < fieldSize*cellSize)
					if (pos.y > 0 && pos.y < fieldSize * cellSize)
					{
						windowMain.requestFocus();
						ptr[pos.x / (windowMain.getSize().x / fieldSize)][pos.y / (windowMain.getSize().y / fieldSize)] = setTo;
					}
			}
			if (event.type == sf::Event::MouseButtonPressed)
			{
				isHelpActive = false;
			}
		}

		//-
		for (int i = 0; i < fieldSize; i++)
			for (int j = 0; j < fieldSize; j++)
			{
				sf::Vertex* quad = &quads[(i + j * fieldSize) * 4];

				quad[0].position = sf::Vector2f(i * cellSize, j * cellSize);
				quad[1].position = sf::Vector2f((i + 1) * cellSize, j * cellSize);
				quad[2].position = sf::Vector2f((i + 1) * cellSize, (j + 1) * cellSize);
				quad[3].position = sf::Vector2f(i * cellSize, (j + 1) * cellSize);

				if (field[i][j]) {
					quad[0].color = sf::Color(210, 110, 110);
					quad[1].color = sf::Color(210, 110, 110);
					quad[2].color = sf::Color(210, 110, 110);
					quad[3].color = sf::Color(210, 110, 110);
				}
				else
				{
					quad[0].color = sf::Color(110, 110, 110);
					quad[1].color = sf::Color(110, 110, 110);
					quad[2].color = sf::Color(110, 110, 110);
					quad[3].color = sf::Color(110, 110, 110);
				}
			}
		//-

		//- buffer
		buffer.update(&quads[0]);
		//- 

		windowMain.draw(buffer);
		if (isHelpActive)
			windowMain.draw(helpText);

		windowMain.display();
	}
}

void showControlPanel()
{
	//sf::ContextSettings settings;
	//settings.antialiasingLevel = 4;

	controlWindow.create(sf::VideoMode(400, 200), "Life control panel", sf::Style::Titlebar | sf::Style::Close);
	controlWindow.clear(sf::Color(255, 230, 185, 0));
	controlWindow.setVerticalSyncEnabled(true);
	controlWindow.setFramerateLimit(120);

	sf::Event event;

	Button LifeStartButton;
	LifeStartButton.load(sf::Vector2u(10, 10), sf::Vector2u(150, 75), sf::Color(170, 170, 170), sf::Color(130, 130, 130), "Start Life", 25, sf::Vector2u(15, 20), &lifeButtonStartPressedEvent);
	Button LifeStopButton;
	LifeStopButton.load(sf::Vector2u(10, 80), sf::Vector2u(150, 145), sf::Color(170, 170, 170), sf::Color(130, 130, 130), "Stop Life", 25, sf::Vector2u(15, 95), &lifeButtonStopPressedEvent);
	Button LoadButton;
	LoadButton.load(sf::Vector2u(170, 90), sf::Vector2u(320, 120), sf::Color(170, 170, 170), sf::Color(130, 130, 130), "Load organism", 16, sf::Vector2u(190, 95), &LoadButtonPressedEvent);
	Button helpButton;
	helpButton.load(sf::Vector2u(170, 10), sf::Vector2u(230, 40), sf::Color(170, 170, 170), sf::Color(130, 130, 130), "help", 18, sf::Vector2u(185, 13), &helpButtonPressedEvent);
	TextBox loadedOrganismNameTextBox;
	loadedOrganismNameTextBox.load(sf::Vector2f(170, 50), sf::Vector2f(320, 80), sf::Color(180, 180, 180), sf::Color(130, 130, 130), "load organism name", 16, sf::Vector2f(-70, -9));
	TextBox simSpeedTextBox;
	simSpeedTextBox.ptrTextBoxEndEvent = &simSpeedTextBoxEndEvent;
	simSpeedTextBox.load(sf::Vector2f(170, 130), sf::Vector2f(320, 160), sf::Color(180, 180, 180), sf::Color(130, 130, 130), "sim speed", 17, sf::Vector2f(-60, -10));
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
	simSpeedTextBox.ableChars = intCharsAble;
	ColorIndicator lifeIndicator1;
	lifeIndicator1.load(sf::Vector2f(140, 10), sf::Vector2f(150, 75), false);
	ColorIndicator lifeIndicator2;
	lifeIndicator2.load(sf::Vector2f(140, 80), sf::Vector2f(150, 145), false);

	while (controlWindow.isOpen())
	{
		loadFileName = loadedOrganismNameTextBox.stringText;
		controlWindow.clear(sf::Color(255, 230, 185, 0));
		while (controlWindow.pollEvent(event))
		{
			// transfer event to exemplar's methods
			LifeStartButton.eventProcces(event);
			LifeStopButton.eventProcces(event);
			LoadButton.eventProcces(event);
			helpButton.eventProcces(event);
			loadedOrganismNameTextBox.eventProces(event);
			simSpeedTextBox.eventProces(event);
			lifeIndicator1.update(isGameLife);
			lifeIndicator2.update(isGameLife);

			if (event.type == sf::Event::Closed) {
				controlWindow.close();
				exit(1);
			}
			//if (event.type == sf::Event::MouseButtonPressed)
				//cout << event.mouseButton.x << " " << event.mouseButton.y << "\n";
		}

		/////////////////// drawing stuff
		controlWindow.draw(LifeStartButton);
		controlWindow.draw(LifeStopButton);
		controlWindow.draw(LoadButton);
		controlWindow.draw(helpButton);
		controlWindow.draw(loadedOrganismNameTextBox);
		controlWindow.draw(lifeIndicator1);
		controlWindow.draw(lifeIndicator2);
		controlWindow.draw(simSpeedTextBox);
		///////////////////

		controlWindow.display();
		Sleep(60);
	}
}

int main()
{
	//HWND hWnd = GetConsoleWindow();
	//ShowWindow(hWnd, SW_HIDE);

	//cout << "Hello World!\n";

	std::thread thr3(turn);
	std::thread thr1(showGraphField, field, 60);
	//thr1.detach();

	Sleep(200);
	std::thread thr(showControlPanel);
	//thr.detach();



	//--------------------------------		temp
	
	//--------------------------------

	std::cin.get();
}
