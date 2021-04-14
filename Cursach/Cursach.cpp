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
#include <iostream>
#include <string.h>
#include <string>
#include <locale>

static const int fieldSize = 128;
static bool field[fieldSize][fieldSize];
static bool isGameLife = false;
static bool isHelpActive = true;
static const float cellSize = 8;
static int simSpeed = 60;
static sf::RenderWindow windowMain;
static sf::RenderWindow controlWindow;
static sf::VertexArray quads;
static sf::VertexBuffer buffer(sf::Quads);
int clockStart;

class Text : public sf::Drawable, public sf::Transformable
{
	sf::Font font;
	sf::Text text;
public:

	Text& operator=(Text other) {
		font = other.font;
		text = other.text;
		text.setFont(font);
		return *this;
	}

	Text(std::string string, int fontSize) {
		if (!font.loadFromFile("arial.ttf")) // Nothing wrong with loading the font
			std::exit(-1);
		text = sf::Text(string, font, fontSize);
	}
	Text() { }

	virtual void draw(sf::RenderTarget & target, sf::RenderStates states) const {
		states.transform *= getTransform();
		target.draw(text, states);
	}
};
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
	bool isPressedPush = false;
	bool isPressedToggle = false;
	bool isTickPush = false;
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
		vertices.setPrimitiveType(sf::PrimitiveType::Quads);
		vertices.resize(4);

		vertices[0].position = sf::Vector2f(firstPoint.x, firstPoint.y);
		vertices[1].position = sf::Vector2f(firstPoint.x, secondPoint.y);
		vertices[2].position = sf::Vector2f(secondPoint.x, secondPoint.y);
		vertices[3].position = sf::Vector2f(secondPoint.x, firstPoint.y);

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

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();	// transforming
		target.draw(vertices, states);		// drawing button rectangle
		target.draw(text);					// drawing text
	}
};
class TextBox : public sf::Drawable, sf::Transformable
{
private:
	sf::VertexArray vertices;	// field of text box
	Text text;					// text of text box
public:
	sf::Color activeColor;		// color when active
	sf::Color noActiveColor;	// color when not active
	std::string stringText;		// string of text in box
	sf::Vector2f firstPoint;
	sf::Vector2f secondPoint;
	int textSize = 16;

	bool isActive = false;

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

		vertices[0].position = firstPoint;
		vertices[1].position = sf::Vector2f(firstPoint.x, secondPoint.y);
		vertices[2].position = secondPoint;
		vertices[3].position = sf::Vector2f(secondPoint.x, firstPoint.y);

		//coloring:
		vertices[0].color = noActiveColor;
		vertices[1].color = noActiveColor;
		vertices[2].color = noActiveColor;
		vertices[3].color = noActiveColor;

		// setting text:
		text = Text(stringText, textSize);
		text.setPosition(((firstPoint.x + secondPoint.x) / 2) + textOffset.x, ((firstPoint.y + secondPoint.y) / 2) + textOffset.y);

		return true;
	}

	void eventProces(sf::Event m_event)
	{
		if (m_event.type == sf::Event::MouseButtonPressed)	// when pressed somewhere NOT on the text box → unActive this shit
		{
			isActive = false;

			vertices[0].color = noActiveColor;
			vertices[1].color = noActiveColor;
			vertices[2].color = noActiveColor;
			vertices[3].color = noActiveColor;
		}
		// When pressed on the text → active this shit
		if (m_event.type == sf::Event::MouseButtonPressed && m_event.mouseButton.x >= firstPoint.x && m_event.mouseButton.y >= firstPoint.y && m_event.mouseButton.x <= secondPoint.x && m_event.mouseButton.y <= secondPoint.y)
		{
			isActive = true;

			vertices[0].color = activeColor;
			vertices[1].color = activeColor;
			vertices[2].color = activeColor;
			vertices[3].color = activeColor;
		}
		if (isActive && m_event.type == sf::Event::TextEntered)		// When active and text keys pressed → write that shit down
		{
			if (m_event.text.unicode == '\b' && stringText != "")
			{
				stringText.erase(stringText.size() - 1, 1);
			}
			if (m_event.text.unicode != '\b' && stringText.size() < 16)
			{
				stringText += static_cast<char>(m_event.text.unicode);
			}
			text = Text(stringText, textSize);
		}
		if (isActive && m_event.type == sf::Event::KeyPressed && m_event.key.code == sf::Keyboard::Enter) // when 'Enter' pressed → unActive this shit
		{
			isActive = false;

			vertices[0].color = noActiveColor;
			vertices[1].color = noActiveColor;
			vertices[2].color = noActiveColor;
			vertices[3].color = noActiveColor;
		}
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();	// transforming
		target.draw(vertices, states);		// drawing button rectangle
		target.draw(text);					// drawing text
	}
};
class ColorIndicator : public sf::Drawable, sf::Transformable
{
private:

	sf::VertexArray vertices;

public:

	sf::Vector2f firstPoint;
	sf::Vector2f secondPoint;
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
	if (pushIn)
	{

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
	helpText.setFont(font);
	helpText.setString("\t\tHELLO! this is Conway's Game of Life!\n\nto start your game you need to draw first generation of cells on gray window\nusing mouse control.\n\nTo calculate next generations you need to press \"Life Button\"\nin another window.\n\nYou can stop simulation anytime you want to draw or remove cells from field.\n\nEnjoy!");

	buffer.create(fieldSize*fieldSize * 4);
	buffer.setUsage(sf::VertexBuffer::Usage::Stream);

	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	windowMain.create(sf::VideoMode(fieldSize*cellSize + 1, fieldSize*cellSize + 1), "Hellow world!", sf::Style::Titlebar | sf::Style::Close, settings);
	windowMain.setFramerateLimit(60);

	windowMain.clear(sf::Color(255, 230, 185, 0));
	sf::Event event;
	while (windowMain.isOpen())
	{
		while (windowMain.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) {
				windowMain.close();
				exit(1);
			}

			if (event.type == sf::Event::MouseButtonPressed && !isGameLife)
			{
				ptr[event.mouseButton.x / (windowMain.getSize().x / fieldSize)][event.mouseButton.y / (windowMain.getSize().y / fieldSize)] = !ptr[event.mouseButton.x / (windowMain.getSize().x / fieldSize)][event.mouseButton.y / (windowMain.getSize().y / fieldSize)];
			}
			if (event.type == sf::Event::MouseButtonPressed)
			{
				isHelpActive = false;
			}
		}

		clockStart = clock();

		quads.setPrimitiveType(sf::Quads);
		quads.resize(fieldSize*fieldSize * 4);

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
	//Button LoadButton;
	//LoadButton.load(sf::Vector2u(10, 85), sf::Vector2u(150, 150), sf::Color(170, 170, 170), sf::Color(130, 130, 130), "Load organism (Not work yet)", 20, sf::Vector2u(15, 95), &LoadButtonPressedEvent);
	//Button RuleButton;
	//RuleButton.load(sf::Vector2u(10, 10), sf::Vector2u(150, 75), sf::Color(170, 170, 170), sf::Color(130, 130, 130), "Life Button", 25, sf::Vector2u(15, 20), &lifeButtonPressedEvent);
	Button helpButton;
	helpButton.load(sf::Vector2u(170, 10), sf::Vector2u(230, 40), sf::Color(170, 170, 170), sf::Color(130, 130, 130), "help", 18, sf::Vector2u(185, 13), &helpButtonPressedEvent);
	TextBox loadedOrganismNameTextBox;
	loadedOrganismNameTextBox.load(sf::Vector2f(170, 50), sf::Vector2f(320, 80), sf::Color(180, 180, 180), sf::Color(130, 130, 130), "load organism name", 16, sf::Vector2f(-70, -9));
	ColorIndicator lifeIndicator1;
	lifeIndicator1.load(sf::Vector2f(140, 10), sf::Vector2f(150, 75), false);
	ColorIndicator lifeIndicator2;
	lifeIndicator2.load(sf::Vector2f(140, 80), sf::Vector2f(150, 145), false);

	while (controlWindow.isOpen())
	{
		controlWindow.clear(sf::Color(255, 230, 185, 0));
		while (controlWindow.pollEvent(event))
		{
			// transfer event to exemplar's methods
			LifeStartButton.eventProcces(event);
			LifeStopButton.eventProcces(event);
			//LoadButton.eventProcces(event);
			//RuleButton.eventProcces(event);
			helpButton.eventProcces(event);
			loadedOrganismNameTextBox.eventProces(event);
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
		//controlWindow.draw(LoadButton);
		controlWindow.draw(helpButton);
		controlWindow.draw(loadedOrganismNameTextBox);
		controlWindow.draw(lifeIndicator1);
		controlWindow.draw(lifeIndicator2);
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
