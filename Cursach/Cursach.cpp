#define NOMINMAX

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

using std::cout;
using std::cin;
static const int fieldSize = 124;
static bool field[fieldSize][fieldSize];
static bool isGameLife = false;
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
	int(*ptrFunction)(bool, bool) = NULL; // pointer to function that calls when button pressed
	bool isPressedPush = false;
	bool isPressedToggle = false;
	std::string buttonText;			// string on button

	// function that calls whith creating an exemplar
	bool load(sf::Vector2u m_firstPoint, sf::Vector2u m_secondPoint, sf::Color m_notPressedColor, sf::Color m_PressedColor, std::string m_buttonText, int fontSize, sf::Vector2u textPosition, int(*ptrFunc)(bool, bool))
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
		vertices.setPrimitiveType(sf::Quads);
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
		// check if pressed on button
		if (m_event.type == sf::Event::MouseButtonPressed && m_event.mouseButton.x >= firstPoint.x && m_event.mouseButton.x <= secondPoint.x && m_event.mouseButton.y >= firstPoint.y && m_event.mouseButton.y <= secondPoint.y)
		{
			isPressedPush = true;
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
		ptrFunction(isPressedPush, isPressedToggle); // call custom function for this button
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();
		target.draw(vertices, states);		// drawing button rectangle
		target.draw(text);					// -------PROBLEM HERE---------
	}
};
int lifeButtonPressedEvent(bool pushIn, bool toggleIn)
{
	if (toggleIn)
		isGameLife = true;
	else
		isGameLife = false;
	return 1;
}
//-------------------------

//-------------------------
int LoadButtonPressedEvent(bool pushIn, bool toggleIn)
{
	if (pushIn)
	{
		
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

void showConsoleField(bool ptr[][fieldSize])
{
	for (int i = 0; i < fieldSize; i++)
	{
		for (int j = 0; j < fieldSize; j++)
		{
			if (ptr[i][j])
				cout << 8 << " ";
			else
				cout << "." << " ";
		}
		cout << "\n";
	}
	cout << "\n";
}

void showGraphField(bool ptr[][fieldSize], int fps)
{
	buffer.create(fieldSize*fieldSize*4);
	buffer.setUsage(sf::VertexBuffer::Usage::Stream);

	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	windowMain.create(sf::VideoMode(fieldSize*cellSize + 1, fieldSize*cellSize + 1), "Hellow world!", sf::Style::Default, settings);
	windowMain.setFramerateLimit(60);

	windowMain.clear(sf::Color(255, 230, 185, 0));
	sf::Event event;
	while (windowMain.isOpen())
	{
		while (windowMain.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				windowMain.close();
			if (event.type == sf::Event::Resized)
				windowMain.setSize(sf::Vector2u(fieldSize * cellSize + 1, fieldSize * cellSize + 1));
			if (event.type == sf::Event::MouseButtonPressed && !isGameLife)
			{
				ptr[event.mouseButton.x / (windowMain.getSize().x / fieldSize)][event.mouseButton.y / (windowMain.getSize().y / fieldSize)] = !ptr[event.mouseButton.x / (windowMain.getSize().x / fieldSize)][event.mouseButton.y / (windowMain.getSize().y / fieldSize)];
			}
		}		

		clockStart = clock();
		
		quads.setPrimitiveType(sf::Quads);
		quads.resize(fieldSize*fieldSize*4);

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
		windowMain.display();
	}
}

void showControlPanel()
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	controlWindow.create(sf::VideoMode(300,200),"Life control panel",sf::Style::Default, settings);
	controlWindow.clear(sf::Color(255, 230, 185, 0));
	controlWindow.setVerticalSyncEnabled(true);
	controlWindow.setFramerateLimit(120);

	sf::Event event;

	Button LifeButton;
	LifeButton.load(sf::Vector2u(10, 10), sf::Vector2u(150, 75), sf::Color(170,170,170), sf::Color(130,130,130), "Life Button", 25, sf::Vector2u(15,20), &lifeButtonPressedEvent);
	Button LoadButton;
	LoadButton.load(sf::Vector2u(10, 85), sf::Vector2u(150, 150), sf::Color(170, 170, 170), sf::Color(130, 130, 130), "Load organism (Not work yet)", 20, sf::Vector2u(15, 95), &LoadButtonPressedEvent);
	Button RuleButton;
	RuleButton.load(sf::Vector2u(10, 10), sf::Vector2u(150, 75), sf::Color(170, 170, 170), sf::Color(130, 130, 130), "Life Button", 25, sf::Vector2u(15, 20), &lifeButtonPressedEvent);

	while (controlWindow.isOpen())
	{
		controlWindow.clear(sf::Color(255, 230, 185, 0));
		while (controlWindow.pollEvent(event))
		{
			// transfer event to exemplar's methods
			LifeButton.eventProcces(event);
			LoadButton.eventProcces(event);
			RuleButton.eventProcces(event);

			if (event.type == sf::Event::Closed)
				controlWindow.close();
			if (event.type == sf::Event::MouseButtonPressed)
				cout << event.mouseButton.x << " " << event.mouseButton.y << "\n";
			if (event.type == sf::Event::Resized)
				controlWindow.setSize(sf::Vector2u(300, 200));
		}

		///////////////////
		controlWindow.draw(LifeButton);
		controlWindow.draw(LoadButton);
		///////////////////

		controlWindow.display();
		Sleep(60);
	}
}

void consoleOutput(int times, int ms)
{
	for (int i = 0; i < times; i++)
	{
		Sleep(ms);

		showConsoleField(field);
		turn();
	}
}

int main()
{
	
	cout << "Hello World!\n";

	std::thread thr3(turn);
	std::thread thr1(showGraphField, field, 60);
	//thr1.detach();

	Sleep(200);
	std::thread thr(showControlPanel);
	//thr.detach();

	cin.get();
	
}
