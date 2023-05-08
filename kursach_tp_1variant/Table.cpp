#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <string>
#undef main

int MAX_DEPTH = 5;


class SDL_PRINT {
public:
    int SIZE_WINDOW = 600;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Event event;
public:

    // Initialize the SDL window and renderer
    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cout << "SDL initialization failed: " << SDL_GetError() << std::endl;
            return false;
        }

        window = SDL_CreateWindow("Checkers", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SIZE_WINDOW, SIZE_WINDOW, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            std::cout << "Window creation failed: " << SDL_GetError() << std::endl;
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == NULL) {
            std::cout << "Renderer creation failed: " << SDL_GetError() << std::endl;
            return false;
        }

        return true;
    }


    void fillTexture(SDL_Renderer* renderer, SDL_Texture* texture, int r, int g, int b, int a)
    {
        SDL_SetRenderTarget(renderer, texture);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderFillRect(renderer, NULL);
    };

    void DrawCircle(SDL_Renderer* renderer, int x, int y, int radius)
    {
        for (int w = 0; w < radius * 2; w++)
        {
            for (int h = 0; h < radius * 2; h++)
            {
                int dx = radius - w; // horizontal offset
                int dy = radius - h; // vertical offset
                if ((dx * dx + dy * dy) <= (radius * radius))
                {
                    SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                }
            }
        }
    };

    void DrawCircleRadius(SDL_Renderer* renderer, int x, int y, int radius)
    {
        for (int w = 0; w < radius * 2; w++)
        {
            for (int h = 0; h < radius * 2; h++)
            {
                int dx = radius - w; // horizontal offset
                int dy = radius - h; // vertical offset
                if ((dx * dx + dy * dy) <= (radius * radius))
                {
                    SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                }
            }
        }
    };

    std::vector<int> GetClickPosition(SDL_Event& event) {

        int x = event.button.x;
        int y = event.button.y;

        int col = y / (SIZE_WINDOW / 8);
        int row = x / (SIZE_WINDOW / 8);
        return { row, col };

    }
};

struct Cell {
    int x;
    int y;
    bool isQueen = false;
    std::string type;
};

struct CellStep {
    int x = -1;
    int y = -1;
    Cell* eatenCell;
};




class Board {
public:
    std::vector<Cell*> checkersCell;
    Cell* currentCell;
    int BOARD_SIZE = 8;
    std::string currentMove = "White";
    std::vector<CellStep> validSteps;
    SDL_PRINT* sdl;

public:

    Board() {
        this->sdl = new SDL_PRINT();
        this->sdl->init();

        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if ((i + j) % 2 == 0) {
                    Cell* emptyCell = new Cell;
                    emptyCell->type = "Empty";
                    emptyCell->x = j;
                    emptyCell->y = i;
                    checkersCell.push_back(emptyCell);
                }
                else if (i > 4) {
                    Cell* whiteCell = new Cell;
                    whiteCell->type = "White";
                    whiteCell->x = j;
                    whiteCell->y = i;
                    /*whiteCell->isQueen = true;*/
                    checkersCell.push_back(whiteCell);
                }
                else if (i < 3) {
                    Cell* blackCell = new Cell;
                    blackCell->type = "Black";
                    blackCell->x = j;
                    blackCell->y = i;
                    /*blackCell->isQueen = true;*/
                    checkersCell.push_back(blackCell);
                }
                else {
                    Cell* emptyCell = new Cell;
                    emptyCell->type = "Empty";
                    emptyCell->x = j;
                    emptyCell->y = i;
                    checkersCell.push_back(emptyCell);
                }
            }
        }
    };

    Board(std::vector<Cell*> cells, Cell* curCell, std::string currentMove, CellStep step, SDL_PRINT* sdlWrapp) {
		try {
			this->sdl = sdlWrapp;
			for (int i = 0; i < cells.size(); i++) {
				Cell* cellPoint = new Cell;
				cellPoint->isQueen = cells[i]->isQueen;
				cellPoint->type = cells[i]->type;
				cellPoint->x = cells[i]->x;
				cellPoint->y = cells[i]->y;

				this->checkersCell.push_back(cellPoint);
				if (cellPoint->x == curCell->x && cellPoint->y == curCell->y) {
					this->currentCell = cellPoint;
				}
			}
			this->currentMove = currentMove;
			if (this->currentCell && this->checkersCell[step.y * 8 + step.x]) {
                this->switchCheckers(this->currentCell, this->checkersCell[step.y * 8 + step.x]);
				if (step.eatenCell) {
					/* std::cout << "Eat: " << step.eatenCell->x << ":" << step.eatenCell->y << std::endl;*/
					this->checkersCell[step.eatenCell->y * 8 + step.eatenCell->x]->type = "Empty";
					this->checkersCell[step.eatenCell->y * 8 + step.eatenCell->x]->isQueen = false;
                    this->currentCell = this->checkersCell[step.y * 8 + step.x];
                    if (this->getValidEatenStep().size() == 0) {
                        this->switchColor();
                        this->currentCell = nullptr;
                    }
                }
                else {
                    this->switchColor();
                    this->currentCell = nullptr;
                }

			}
            
		}
        catch(...) {
            std::cout << "ошибка в конструкторе: "  << std::endl;
        }
    }

    ~Board() {
        this->checkersCell.clear();
        this->currentCell = nullptr;
    }

    void printBoard() {
            for (int i = 0; i < this->checkersCell.size(); i++) {
                if (i % 8 == 0) {
                    std::cout << std::endl;
                }
                char sym = '#';
                if (this->checkersCell[i]->type == "Black") {
                    if (this->checkersCell[i]->isQueen) {
                        sym = 'B';
                    }
                    else
                        sym = 'b';
                }
                else if (this->checkersCell[i]->type == "White") {
                    if (this->checkersCell[i]->isQueen) {
                        sym = 'W';
                    }
                    else
                        sym = 'w';
                }
                std::cout << sym;
            }
            std::cout << std::endl;
            std::cout << "==================================" << std::endl;
    }

    void switchColor() {
        if (this->currentMove == "White") {
            this->currentMove = "Black";
        }
        else if (this->currentMove == "Black") {
            this->currentMove = "White";
        }
    };

    int isTheEndGame() {
        int white = 0;
        int black = 0;
        
        for (int i = 0; i < this->checkersCell.size(); i++) {
            if (this->checkersCell[i]->type == "Black") {
                black++;
            }
            else if (this->checkersCell[i]->type == "White") {
                white++;
            }

        }

        if (white && black) {
            return 0;
        }
        else if (white && !black) {
            return 1;
        }
        else if (!white && black) {
            return -1;
        }
    }


    void switchCheckers(Cell* first, Cell* second) {
        Cell temp;
        temp.isQueen = first->isQueen;
        temp.type = first->type;

        Cell temp2;
        temp2.isQueen = second->isQueen;
        temp2.type = second->type;

        first->isQueen = temp2.isQueen;
        first->type = temp2.type;
        second->isQueen = temp.isQueen;
        second->type = temp.type;

        if (first->type == "White" && first->y == 0) {
            first->isQueen = true;
        }
        else if (first->type == "Black" && first->y == 7) {
            first->isQueen = true;
        }
        else if (second->type == "White" && second->y == 0) {
            second->isQueen = true;
        }
        else if (second->type == "Black" && second->y == 7) {
            second->isQueen = true;
        }
    };

    void drawBoard() {
        for (int i = 0; i < checkersCell.size(); i++) {
            SDL_Rect rect;
            rect.x = checkersCell.at(i)->x * 75;
            rect.y = checkersCell.at(i)->y * 75;
            rect.w = 75;
            rect.h = 75;
            if ((checkersCell.at(i)->x + checkersCell.at(i)->y) % 2 == 0) {
                SDL_SetRenderDrawColor(sdl->renderer, 220, 181, 120, 255);
            }
            else {
                SDL_SetRenderDrawColor(sdl->renderer, 107, 48, 34, 255);
            }
            SDL_RenderFillRect(sdl->renderer, &rect);
            if (checkersCell.at(i)->type == "Black") {

                SDL_SetRenderDrawColor(sdl->renderer, 13, 12, 6, 255);
                sdl->DrawCircle(sdl->renderer, rect.x + 37, rect.y + 37, 30);

                SDL_SetRenderDrawColor(sdl->renderer, 66, 66, 66, 255);
                sdl->DrawCircleRadius(sdl->renderer, rect.x + 37, rect.y + 37, 20);

                SDL_SetRenderDrawColor(sdl->renderer, 13, 12, 6, 255);
                sdl->DrawCircleRadius(sdl->renderer, rect.x + 37, rect.y + 37, 10);
            }
            else if (checkersCell.at(i)->type == "White") {

                SDL_SetRenderDrawColor(sdl->renderer, 177, 163, 125, 255);
                sdl->DrawCircle(sdl->renderer, rect.x + 37, rect.y + 37, 30);

                SDL_SetRenderDrawColor(sdl->renderer, 171, 131, 108, 255);
                sdl->DrawCircleRadius(sdl->renderer, rect.x + 37, rect.y + 37, 20);

                SDL_SetRenderDrawColor(sdl->renderer, 177, 163, 125, 255);
                sdl->DrawCircleRadius(sdl->renderer, rect.x + 37, rect.y + 37, 10);
            };

            if (checkersCell.at(i)->isQueen) {
                SDL_SetRenderDrawColor(sdl->renderer, 200, 200, 125, 255);
                sdl->DrawCircleRadius(sdl->renderer, rect.x + 37, rect.y + 37, 18);
            };
        };

        if (this->currentCell) {

            SDL_Rect rect;
            rect.x = this->currentCell->x * 75;
            rect.y = this->currentCell->y * 75;
            rect.w = 75;
            rect.h = 75;

            SDL_SetRenderDrawBlendMode(sdl->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(sdl->renderer, 207, 207, 207, 170);
            SDL_RenderFillRect(sdl->renderer, &rect);

            for (int i = 0; i < this->getAllValidSteps().size(); i++) {
                SDL_SetRenderDrawColor(this->sdl->renderer, 68, 148, 74, 255);
                this->sdl->DrawCircleRadius(sdl->renderer, 75 * this->getAllValidSteps()[i].x + 37, 75 * this->getAllValidSteps()[i].y + 37, 10);
            }

        };
        SDL_RenderPresent(sdl->renderer);
    };

    std::vector<CellStep> getValidEmptyStep() {

        int x = this->currentCell->x;
        int y = this->currentCell->y;

        std::vector<CellStep> validSteps;

        if (!this->currentCell->isQueen) {
            if (this->currentCell->type == "White") {

                //узнаем, не левая крайняя ли это точка у доски, входит ли в диапозон точек на доске
                //для простого хода влево по диаганали
                if (y * this->BOARD_SIZE + x - 9 >= 0
                    &&
                    ((y * this->BOARD_SIZE) + x - 9 < this->BOARD_SIZE * this->BOARD_SIZE)
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x - 9]->type == "Empty"
                    &&
                    x != 0) {
                    CellStep step;
                    step.x = this->checkersCell[y * this->BOARD_SIZE + x - 9]->x;
                    step.y = this->checkersCell[y * this->BOARD_SIZE + x - 9]->y;
                    step.eatenCell = nullptr;
                    validSteps.push_back(step);
                }

                //для простого хода вправо по диаганали
                if (y * this->BOARD_SIZE + x - 7 >= 0
                    &&
                    ((y * this->BOARD_SIZE) + x - 7 < this->BOARD_SIZE * this->BOARD_SIZE)
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x - 7]->type == "Empty"
                    &&
                    x != this->BOARD_SIZE - 1) {
                    CellStep step;
                    step.x = this->checkersCell[y * this->BOARD_SIZE + x - 7]->x;
                    step.y = this->checkersCell[y * this->BOARD_SIZE + x - 7]->y;
                    step.eatenCell = nullptr;
                    validSteps.push_back(step);

                }
            };

            if (this->currentCell->type == "Black") {

                //узнаем, не правая крайняя ли это точка у доски, входит ли в диапозон точек на доске
                //для простого хода вправо по диаганали
                if (y * this->BOARD_SIZE + x + 9 >= 0
                    &&
                    ((y * this->BOARD_SIZE) + x + 9 < this->BOARD_SIZE * this->BOARD_SIZE)
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x + 9]->type == "Empty"
                    &&
                    x != this->BOARD_SIZE - 1) {
                    CellStep step;
                    step.x = this->checkersCell[y * this->BOARD_SIZE + x + 9]->x;
                    step.y = this->checkersCell[y * this->BOARD_SIZE + x + 9]->y;
                    step.eatenCell = nullptr;
                    validSteps.push_back(step);
                }

                //для простого хода влево по диаганали
                if (y * this->BOARD_SIZE + x + 7 >= 0
                    &&
                    ((y * this->BOARD_SIZE) + x + 7 < this->BOARD_SIZE * this->BOARD_SIZE)
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x + 7]->type == "Empty"
                    &&
                    x != 0) {
                    CellStep step;
                    step.x = this->checkersCell[y * this->BOARD_SIZE + x + 7]->x;
                    step.y = this->checkersCell[y * this->BOARD_SIZE + x + 7]->y;
                    step.eatenCell = nullptr;
                    validSteps.push_back(step);
                }
            }
        };
        if (this->currentCell->isQueen) {
            if (this->currentCell->type == "White") {
                //==========================================//
                //  для диагонали \
                //                 \ вниз
                //                  \

                for (int i = 1;
                    i + x <= 7
                    && i + y <= 7
                    && (y * this->BOARD_SIZE + x + i * (this->BOARD_SIZE + 1) < this->BOARD_SIZE * this->BOARD_SIZE);
                    i++) {
                    int indexElement = (y * this->BOARD_SIZE + x);
                    //ищем на диагонали черную пешку
                    if (this->checkersCell[indexElement + i * (this->BOARD_SIZE + 1)]->type == "Empty") {
                        CellStep step;
                        step.x = checkersCell[indexElement + i * (this->BOARD_SIZE + 1)]->x;
                        step.y = checkersCell[indexElement + i * (this->BOARD_SIZE + 1)]->y;
                        step.eatenCell = nullptr;
                        validSteps.push_back(step);
                    }
                    else {
                        //при не пустой клетку завершаем поиск
                        break;
                    }
                }
                //==========================================//
                 //  для диагонали \
                //                  \ вверх
                //                   \
                
                for (int i = 1;
                    x - i >= 0
                    && y - i >= 0
                    && (y * this->BOARD_SIZE + x - i * (this->BOARD_SIZE + 1) >= 0);
                    i++) {
                    int indexElement = (y * this->BOARD_SIZE + x);
                    //ищем на диагонали черную пешку
                    if (this->checkersCell[indexElement - i * (this->BOARD_SIZE + 1)]->type == "Empty") {
                        CellStep step;
                        step.x = checkersCell[indexElement - i * (this->BOARD_SIZE + 1)]->x;
                        step.y = checkersCell[indexElement - i * (this->BOARD_SIZE + 1)]->y;
                        step.eatenCell = nullptr;
                        validSteps.push_back(step);
                    }
                    else {
                        break;
                    }
                }
                //==========================================//
                 //  для диагонали   /
                 //                 / вниз
                 //                /

                for (int i = 1;
                    x - i >= 0
                    && i + y <= 7
                    && (y * this->BOARD_SIZE + x + i * (this->BOARD_SIZE - 1) < this->BOARD_SIZE * this->BOARD_SIZE);
                    i++) {
                    int indexElement = (y * this->BOARD_SIZE + x);
                    //ищем на диагонали черную пешку
                    if (this->checkersCell[indexElement + i * (this->BOARD_SIZE - 1)]->type == "Empty") {
                        CellStep step;
                        step.x = checkersCell[indexElement + i * (this->BOARD_SIZE - 1)]->x;
                        step.y = checkersCell[indexElement + i * (this->BOARD_SIZE - 1)]->y;
                        step.eatenCell = nullptr;
                        validSteps.push_back(step);
                    }
                    else {
                        //при не пустой клетку завершаем поиск
                        break;
                    }
                }
                //==========================================//
                //  для диагонали   /
                //                 / вверх
                //                /

                for (int i = 1;
                    x + i <= 7
                    && y - i >= 0
                    && (y * this->BOARD_SIZE + x - i * (this->BOARD_SIZE - 1) >= 0);
                    i++) {
                    int indexElement = (y * this->BOARD_SIZE + x);
                    //ищем на диагонали черную пешку
                    if (this->checkersCell[indexElement - i * (this->BOARD_SIZE - 1)]->type == "Empty") {
                        CellStep step;
                        step.x = checkersCell[indexElement - i * (this->BOARD_SIZE - 1)]->x;
                        step.y = checkersCell[indexElement - i * (this->BOARD_SIZE - 1)]->y;
                        step.eatenCell = nullptr;
                        validSteps.push_back(step);
                    }
                    else {
                        break;
                    }
                }
            }

            if (this->currentCell->type == "Black") {
                //==========================================//
                //  для диагонали \
                //                 \ вниз
                //                  \

                for (int i = 1;
                    i + x <= 7
                    && i + y <= 7
                    && (y * this->BOARD_SIZE + x + i * (this->BOARD_SIZE + 1) < this->BOARD_SIZE * this->BOARD_SIZE);
                    i++) {
                    int indexElement = (y * this->BOARD_SIZE + x);
                    //ищем на диагонали черную пешку
                    if (this->checkersCell[indexElement + i * (this->BOARD_SIZE + 1)]->type == "Empty") {
                        CellStep step;
                        step.x = checkersCell[indexElement + i * (this->BOARD_SIZE + 1)]->x;
                        step.y = checkersCell[indexElement + i * (this->BOARD_SIZE + 1)]->y;
                        step.eatenCell = nullptr;
                        validSteps.push_back(step);
                    }
                    else {
                        //при не пустой клетку завершаем поиск
                        break;
                    }
                }
                //==========================================//
                 //  для диагонали \
                //                  \ вверх
                //                   \
                
                for (int i = 1;
                    x - i >= 0
                    && y - i >= 0
                    && (y * this->BOARD_SIZE + x - i * (this->BOARD_SIZE + 1) >= 0);
                    i++) {
                    int indexElement = (y * this->BOARD_SIZE + x);
                    //ищем на диагонали черную пешку
                    if (this->checkersCell[indexElement - i * (this->BOARD_SIZE + 1)]->type == "Empty") {
                        CellStep step;
                        step.x = checkersCell[indexElement - i * (this->BOARD_SIZE + 1)]->x;
                        step.y = checkersCell[indexElement - i * (this->BOARD_SIZE + 1)]->y;
                        step.eatenCell = nullptr;
                        validSteps.push_back(step);
                    }
                    else {
                        break;
                    }
                }
                //==========================================//
                //  для диагонали   /
                //                 / вниз
                //                /

                for (int i = 1;
                    x - i >= 0
                    && i + y <= 7
                    && (y * this->BOARD_SIZE + x + i * (this->BOARD_SIZE - 1) < this->BOARD_SIZE * this->BOARD_SIZE);
                    i++) {
                    int indexElement = (y * this->BOARD_SIZE + x);
                    //ищем на диагонали черную пешку
                    if (this->checkersCell[indexElement + i * (this->BOARD_SIZE - 1)]->type == "Empty") {
                        CellStep step;
                        step.x = checkersCell[indexElement + i * (this->BOARD_SIZE - 1)]->x;
                        step.y = checkersCell[indexElement + i * (this->BOARD_SIZE - 1)]->y;
                        step.eatenCell = nullptr;
                        validSteps.push_back(step);
                    }
                    else {
                        //при не пустой клетку завершаем поиск
                        break;
                    }
                }
                //==========================================//
                //  для диагонали   /
                //                 / вверх
                //                /

                for (int i = 1;
                    x + i <= 7
                    && y - i >= 0
                    && (y * this->BOARD_SIZE + x - i * (this->BOARD_SIZE - 1) >= 0);
                    i++) {
                    int indexElement = (y * this->BOARD_SIZE + x);
                    //ищем на диагонали черную пешку
                    if (this->checkersCell[indexElement - i * (this->BOARD_SIZE - 1)]->type == "Empty") {
                        CellStep step;
                        step.x = checkersCell[indexElement - i * (this->BOARD_SIZE - 1)]->x;
                        step.y = checkersCell[indexElement - i * (this->BOARD_SIZE - 1)]->y;
                        step.eatenCell = nullptr;
                        validSteps.push_back(step);
                    }
                    else {
                        break;
                    }
                }
            }

        }

        return validSteps;
    };

    std::vector<CellStep> getValidEatenStep() {
        int x = this->currentCell->x;
        int y = this->currentCell->y;

        std::vector<CellStep> validSteps;

        if (!this->currentCell->isQueen) {
            if (this->currentCell->type == "White") {
                //для хода, который съедает левую пешку по диагонали
                if (y * this->BOARD_SIZE + x - 18 >= 0
                    &&
                    ((y * this->BOARD_SIZE) + x - 18 < this->BOARD_SIZE * this->BOARD_SIZE)
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x - 9]->type == "Black"
                    &&
                    x >= 2
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x - 18]->type == "Empty"
                    ) {
                    CellStep step;
                    step.x = this->checkersCell[y * this->BOARD_SIZE + x - 18]->x;
                    step.y = this->checkersCell[y * this->BOARD_SIZE + x - 18]->y;
                    step.eatenCell = this->checkersCell[y * this->BOARD_SIZE + x - 9];
                    validSteps.push_back(step);
                }

                //для хода, который съедает правую пешку по диагонали
                if (y * this->BOARD_SIZE + x - 14 >= 0
                    &&
                    ((y * this->BOARD_SIZE) + x - 14 < this->BOARD_SIZE * this->BOARD_SIZE)
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x - 7]->type == "Black"
                    &&
                    x <= 5
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x - 14]->type == "Empty"
                    ) {
                    CellStep step;
                    step.x = this->checkersCell[y * this->BOARD_SIZE + x - 14]->x;
                    step.y = this->checkersCell[y * this->BOARD_SIZE + x - 14]->y;
                    step.eatenCell = this->checkersCell[y * this->BOARD_SIZE + x - 7];
                    validSteps.push_back(step);
                }

                //для хода, который съедает левую пешку по диагонали вниз
                if (y * this->BOARD_SIZE + x + 18 >= 0
                    &&
                    ((y * this->BOARD_SIZE) + x + 18 < this->BOARD_SIZE * this->BOARD_SIZE)
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x + 9]->type == "Black"
                    &&
                    x <= 5
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x + 18]->type == "Empty"
                    ) {
                    CellStep step;
                    step.x = this->checkersCell[y * this->BOARD_SIZE + x + 18]->x;
                    step.y = this->checkersCell[y * this->BOARD_SIZE + x + 18]->y;
                    step.eatenCell = this->checkersCell[y * this->BOARD_SIZE + x + 9];
                    validSteps.push_back(step);
                }

                //для хода, который съедает правую пешку по диагонали
                if (y * this->BOARD_SIZE + x + 14 >= 0
                    &&
                    ((y * this->BOARD_SIZE) + x + 14 < this->BOARD_SIZE * this->BOARD_SIZE)
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x + 7]->type == "Black"
                    &&
                    x >= 2
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x + 14]->type == "Empty"
                    ) {
                    CellStep step;
                    step.x = this->checkersCell[y * this->BOARD_SIZE + x + 14]->x;
                    step.y = this->checkersCell[y * this->BOARD_SIZE + x + 14]->y;
                    step.eatenCell = this->checkersCell[y * this->BOARD_SIZE + x + 7];
                    validSteps.push_back(step);
                }

            }

            if (this->currentCell->type == "Black") {
                //для хода, который съедает правую пешку по диагонали
                if (y * this->BOARD_SIZE + x + 18 >= 0
                    &&
                    ((y * this->BOARD_SIZE) + x + 18 < this->BOARD_SIZE * this->BOARD_SIZE)
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x + 9]->type == "White"
                    &&
                    x <= 5
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x + 18]->type == "Empty"
                    ) {
                    CellStep step;
                    step.x = this->checkersCell[y * this->BOARD_SIZE + x + 18]->x;
                    step.y = this->checkersCell[y * this->BOARD_SIZE + x + 18]->y;
                    step.eatenCell = this->checkersCell[y * this->BOARD_SIZE + x + 9];
                    validSteps.push_back(step);
                }

                //для хода, который съедает левую пешку по диагонали
                if (y * this->BOARD_SIZE + x + 14 >= 0
                    &&
                    ((y * this->BOARD_SIZE) + x + 14 < this->BOARD_SIZE * this->BOARD_SIZE)
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x + 7]->type == "White"
                    &&
                    x >= 2
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x + 14]->type == "Empty"
                    ) {
                    CellStep step;
                    step.x = this->checkersCell[y * this->BOARD_SIZE + x + 14]->x;
                    step.y = this->checkersCell[y * this->BOARD_SIZE + x + 14]->y;
                    step.eatenCell = this->checkersCell[y * this->BOARD_SIZE + x + 7];
                    validSteps.push_back(step);
                }

                //для хода, который съедает правую пешку по диагонали вниз
                if (y * this->BOARD_SIZE + x - 18 >= 0
                    &&
                    ((y * this->BOARD_SIZE) + x - 18 < this->BOARD_SIZE * this->BOARD_SIZE)
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x - 9]->type == "White"
                    &&
                    x >= 2
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x - 18]->type == "Empty"
                    ) {
                    CellStep step;
                    step.x = this->checkersCell[y * this->BOARD_SIZE + x - 18]->x;
                    step.y = this->checkersCell[y * this->BOARD_SIZE + x - 18]->y;
                    step.eatenCell = this->checkersCell[y * this->BOARD_SIZE + x - 9];
                    validSteps.push_back(step);
                }

                //для хода, который съедает левую пешку по диагонали вниз
                if (y * this->BOARD_SIZE + x - 14 >= 0
                    &&
                    ((y * this->BOARD_SIZE) + x - 14 < this->BOARD_SIZE * this->BOARD_SIZE)
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x - 7]->type == "White"
                    &&
                    x <= 5
                    &&
                    this->checkersCell[y * this->BOARD_SIZE + x - 14]->type == "Empty"
                    ) {
                    CellStep step;
                    step.x = this->checkersCell[y * this->BOARD_SIZE + x - 14]->x;
                    step.y = this->checkersCell[y * this->BOARD_SIZE + x - 14]->y;
                    step.eatenCell = this->checkersCell[y * this->BOARD_SIZE + x - 7];
                    validSteps.push_back(step);
                }
            }
        }
        if (this->currentCell->isQueen) {
            if (this->currentCell->type == "White" || this->currentCell->type == "Black") {
                //==========================================//
                //  для диагонали \
                //                 \ вниз
                //                  \

                for (int i = 1;
                    i + x <= 6
                    && i + y <= 6
                    && (y * this->BOARD_SIZE + x + i * (this->BOARD_SIZE + 1) < this->BOARD_SIZE * this->BOARD_SIZE);
                    i++) {
                    int indexElement = (y * this->BOARD_SIZE + x);
                    //ищем на диагонали черную пешку
                    if (this->checkersCell[indexElement + i * (this->BOARD_SIZE + 1)]->type == "Black" && this->currentCell->type == "White"
                        || this->checkersCell[indexElement + i * (this->BOARD_SIZE + 1)]->type == "White" && this->currentCell->type == "Black") {
                        for (int j = i + 1;
                            j + x <= 7
                            && j + y <= 7
                            && (y * this->BOARD_SIZE + x + j * (this->BOARD_SIZE + 1) < this->BOARD_SIZE * this->BOARD_SIZE);
                            j++) {
                            //ищем пустую клетку
                            int indexCheckerPos = (y * this->BOARD_SIZE + x + j * (this->BOARD_SIZE + 1));
                            if (this->checkersCell[indexCheckerPos]->type == "Empty") {
                                CellStep step;
                                step.x = this->checkersCell[indexCheckerPos]->x;
                                step.y = this->checkersCell[indexCheckerPos]->y;
                                step.eatenCell = this->checkersCell[indexElement + i * (this->BOARD_SIZE + 1)];
                                validSteps.push_back(step);
                            }
                            else {
                                //если встречаем не пустую клетку, то завершаем поиск, тк дальнейшей возможности сьесть нет
                                break;
                            }
                        }
                        //прекращаем дальнейшие перебор, тк больше нет возможности сьесть еще пешку за раз
                        break;
                    }
                    else if (this->checkersCell[indexElement + i * (this->BOARD_SIZE + 1)]->type != "Empty") {
                        break;
                    }
                }
                //==========================================//
                //  для диагонали \
                //                 \ вверх
                //                  \

                for (int i = 1;
                    x - i >= 1
                    && y - i >= 1
                    && (y * this->BOARD_SIZE + x - i * (this->BOARD_SIZE + 1) >= 0);
                    i++) {
                    int indexElement = (y * this->BOARD_SIZE + x);
                    //ищем на диагонали черную пешку
                    if (this->checkersCell[indexElement - i * (this->BOARD_SIZE + 1)]->type == "Black" && this->currentCell->type == "White"
                        || this->checkersCell[indexElement - i * (this->BOARD_SIZE + 1)]->type == "White" && this->currentCell->type == "Black") {
                        for (int j = i + 1;
                            x - j >= 0
                            && y - j >= 0
                            && (y * this->BOARD_SIZE + x - j * (this->BOARD_SIZE + 1) >= 0);
                            j++) {
                            //ищем пустую клетку
                            int indexCheckerPos = (y * this->BOARD_SIZE + x - j * (this->BOARD_SIZE + 1));
                            if (this->checkersCell[indexCheckerPos]->type == "Empty") {
                                CellStep step;
                                step.x = this->checkersCell[indexCheckerPos]->x;
                                step.y = this->checkersCell[indexCheckerPos]->y;
                                step.eatenCell = this->checkersCell[indexElement - i * (this->BOARD_SIZE + 1)];
                                validSteps.push_back(step);
                            }
                            else {
                                //если встречаем не пустую клетку, то завершаем поиск, тк дальнейшей возможности сьесть нет
                                break;
                            }
                        }
                        //прекращаем дальнейшие перебор, тк больше нет возможности сьесть еще пешку за раз
                        break;
                    }
                    else if (this->checkersCell[indexElement - i * (this->BOARD_SIZE + 1)]->type != "Empty") {
                        break;
                    }
                }
                //==========================================//
                //  для диагонали   /
                //                 / вниз
                //                /

                for (int i = 1;
                    x - i >= 1
                    && y + i <= 6
                    && (y * this->BOARD_SIZE + x + i * (this->BOARD_SIZE - 1) < this->BOARD_SIZE * this->BOARD_SIZE);
                    i++) {
                    int indexElement = (y * this->BOARD_SIZE + x);
                    //ищем на диагонали черную пешку
                    if (this->checkersCell[indexElement + i * (this->BOARD_SIZE - 1)]->type == "Black" && this->currentCell->type == "White"
                        || this->checkersCell[indexElement + i * (this->BOARD_SIZE - 1)]->type == "White" && this->currentCell->type == "Black") {
                        for (int j = i + 1;
                            x - j >= 0
                            && y + j <= 7
                            && (y * this->BOARD_SIZE + x + j * (this->BOARD_SIZE - 1) >= 0);
                            j++) {
                            //ищем пустую клетку
                            int indexCheckerPos = (y * this->BOARD_SIZE + x + j * (this->BOARD_SIZE - 1));
                            if (this->checkersCell[indexCheckerPos]->type == "Empty") {
                                CellStep step;
                                step.x = this->checkersCell[indexCheckerPos]->x;
                                step.y = this->checkersCell[indexCheckerPos]->y;
                                step.eatenCell = this->checkersCell[indexElement + i * (this->BOARD_SIZE - 1)];
                                validSteps.push_back(step);
                            }
                            else {
                                //если встречаем не пустую клетку, то завершаем поиск, тк дальнейшей возможности сьесть нет
                                break;
                            }
                        }
                        //прекращаем дальнейшие перебор, тк больше нет возможности сьесть еще пешку за раз
                        break;
                    }
                    else if (this->checkersCell[indexElement + i * (this->BOARD_SIZE - 1)]->type != "Empty") {
                        break;
                    }
                }
                //==========================================//
                //  для диагонали   /
                //                 / вверх
                //                /

                for (int i = 1;
                    x + i <= 6
                    && y - i >= 1
                    && (y * this->BOARD_SIZE + x - i * (this->BOARD_SIZE - 1) >= 0);
                    i++) {
                    int indexElement = (y * this->BOARD_SIZE + x);
                    //ищем на диагонали черную пешку
                    if (this->checkersCell[indexElement - i * (this->BOARD_SIZE - 1)]->type == "Black" && this->currentCell->type == "White"
                        || this->checkersCell[indexElement - i * (this->BOARD_SIZE - 1)]->type == "White" && this->currentCell->type == "Black") {
                        for (int j = i + 1;
                            x + j <= 7
                            && y - j >= 0
                            && (y * this->BOARD_SIZE + x - j * (this->BOARD_SIZE - 1) >= 0);
                            j++) {
                            //ищем пустую клетку
                            int indexCheckerPos = (y * this->BOARD_SIZE + x - j * (this->BOARD_SIZE - 1));
                            if (this->checkersCell[indexCheckerPos]->type == "Empty") {
                                CellStep step;
                                step.x = this->checkersCell[indexCheckerPos]->x;
                                step.y = this->checkersCell[indexCheckerPos]->y;
                                step.eatenCell = this->checkersCell[indexElement - i * (this->BOARD_SIZE - 1)];
                                validSteps.push_back(step);
                            }
                            else {
                                //если встречаем не пустую клетку, то завершаем поиск, тк дальнейшей возможности сьесть нет
                                break;
                            }
                        }
                        //прекращаем дальнейшие перебор, тк больше нет возможности сьесть еще пешку за раз
                        break;
                    }
                    else if (this->checkersCell[indexElement - i * (this->BOARD_SIZE - 1)]->type != "Empty") {
                        break;
                    }
                }
            }

        }

        return validSteps;
    }

    std::vector<CellStep> getAllValidSteps() {
        std::vector<CellStep> emptySteps = this->getValidEmptyStep();
        std::vector<CellStep> eatenSteps = this->getValidEatenStep();
        std::vector<CellStep> allSteps = eatenSteps.size() > 0 ? eatenSteps : emptySteps;
        return allSteps;
    }
};

// оценочная функция
int evaluation_function(Board* board)
{
    int checkers_computer = 0;
    int checkers_player = 0;
    for (int i = 0; i < board->checkersCell.size(); i++) {
        /*checkers_computer+=*/
        if (board->checkersCell[i]->type == "Black") {
            checkers_computer += board->checkersCell[i]->isQueen ? 3 : 1;
        }
        else if (board->checkersCell[i]->type == "White") {
            checkers_player += board->checkersCell[i]->isQueen ? 3 : 1;
        }
    };

    return (checkers_computer - checkers_player);
};


//алгоритм минимакса
int minMaxAlgorithm(Board* curBoard, int depth) {
	try {
		if (depth > MAX_DEPTH || curBoard->isTheEndGame() == -1 || curBoard->isTheEndGame() == 1) {
            
            
			return evaluation_function(curBoard);
		}

		int bestScore = -99999;
        int minEval = 99999;

        if (curBoard->currentCell) {
            std::vector<CellStep> steps = curBoard->getValidEatenStep();
            for (int step = 0; step < steps.size(); step++) {
                //создаем виртуальную доску с выбранным ходом
                Board* newBoard = new Board(curBoard->checkersCell, curBoard->currentCell, curBoard->currentMove, steps[step], curBoard->sdl);
                int newValue = minMaxAlgorithm(newBoard, depth + 1);

                if (curBoard->currentCell->type == "Black") {
                    if (newValue > bestScore) {
                        bestScore = newValue;
                    }
                }
                else if (curBoard->currentCell->type == "White") {
                    if (newValue < minEval) {
                        minEval = newValue;
                    }
                }

                newBoard->~Board();
                delete newBoard;
            }
        }
        else {

            //пробегаемся по всем клеткам доски
            for (int cellInd = 0; cellInd < curBoard->checkersCell.size(); cellInd++) {
                //выбираем только пешки текущего цвета
                if (curBoard->checkersCell[cellInd]->type != curBoard->currentMove) {
                    continue;
                }
                //выбираем пешку как текущую и ищем доступные ходы 
                curBoard->currentCell = curBoard->checkersCell[cellInd];

                std::vector<CellStep> steps = curBoard->getAllValidSteps();
                for (int step = 0; step < steps.size(); step++) {
                    //создаем виртуальную доску с выбранным ходом
                    Board* newBoard = new Board(curBoard->checkersCell, curBoard->currentCell, curBoard->currentMove, steps[step], curBoard->sdl);
                    int newValue = minMaxAlgorithm(newBoard, depth + 1);

                   /* if ((curBoard->currentCell->type == "Black" ? 1 : -1) * newValue > bestScore) {
                        bestScore = (curBoard->currentCell->type == "Black" ? 1 : -1) * newValue;
                    }*/
                    if (curBoard->currentCell->type == "Black") {
                        if (newValue > bestScore) {
                            bestScore = newValue;
                        }
                    }
                    else if (curBoard->currentCell->type == "White") {
                        if (newValue < minEval) {
                            minEval = newValue;
                        }
                    }

                    newBoard->~Board();
                    delete newBoard;
                }

            }
        }

		return curBoard->currentMove == "Black" ? bestScore : minEval;
	}
    catch (...) {
        std::cout << "MinMaxError: " << std::endl;
    }
}




int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    Board* board = new Board();
    bool quit = false;
    bool isEnd = false;
    SDL_Event event;

    try {


        while (!quit)
        {
            //отрисовка доски
            board->drawBoard();
            if (board->isTheEndGame() != 0) {
                std::cout << "êîíåö èãðû - ïîáåäåëè";
                board->isTheEndGame() == 1 ? std::cout << " áåëûå" : std::cout << " ÷åðíûå";
                isEnd = true;
                break;
            }


            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    quit = true;
                }
                //событие клик
                else if (event.type == SDL_MOUSEBUTTONUP) {
                    std::vector<int> curPos = board->sdl->GetClickPosition(event);
                    //цвет выбранного поля совпадает с текущим ходом
                    if (!board->currentCell && board->checkersCell[curPos[1] * 8 + curPos[0]]->type == board->currentMove && board->currentMove == "White") {
                        board->currentCell = board->checkersCell[8 * curPos[1] + curPos[0]];
                    }
                    else if (board->currentCell && board->currentMove == "White") {
                        bool isMadeMove = false;
                        //если у текущей точки отсутствуют ходы, которые будут поедать другие пешки
                        if (board->getValidEatenStep().size() == 0) {
                            std::vector<CellStep> accessPosition = board->getValidEmptyStep();
                            //ищем поле, если оно доступно - делаем ход
                            for (int i = 0; i < accessPosition.size(); i++) {
                                if (curPos[0] == accessPosition[i].x && curPos[1] == accessPosition[i].y) {

                                    board->switchCheckers(board->currentCell, board->checkersCell[curPos[1] * 8 + curPos[0]]);

                                    board->currentCell = nullptr;
                                    board->switchColor();
                                    isMadeMove = true;
                                    break;
                                }
                            }
                        }

                        //если у текущей точки есть ходы, которые сьедают пешку
                        else if (board->getValidEatenStep().size() > 0) {
                            std::vector<CellStep> accessPosition = board->getValidEatenStep();
                            //ищем поле, если оно доступно - делаем ход
                            for (int i = 0; i < accessPosition.size(); i++) {
                                if (curPos[0] == accessPosition[i].x && curPos[1] == accessPosition[i].y) {

                                    accessPosition[i].eatenCell->type = "Empty";
                                    accessPosition[i].eatenCell->isQueen = false;

                                    board->switchCheckers(board->currentCell, board->checkersCell[curPos[1] * 8 + curPos[0]]);
                                    board->currentCell = board->checkersCell[curPos[1] * 8 + curPos[0]];
                                    //если в такой позиции текущая точка не может сьесть еще пешку, то ход переходит другому цвету
                                    if (board->getValidEatenStep().size() == 0) {
                                        board->currentCell = nullptr;
                                        board->switchColor();
                                    }
                                    isMadeMove = true;
                                    break;
                                }
                            }
                            //тк если пешка может сьесть другую, она обязана это сделать, выбрать другую нельзя
                            continue;
                        }

                        //если просто переключение на другую пешку того что цвета, при условии, что ход не сделан
                        if (board->checkersCell[curPos[1] * 8 + curPos[0]]->type == board->currentMove && !isMadeMove) {
                            board->currentCell = board->checkersCell[curPos[1] * 8 + curPos[0]];
                        }
                    }


                    //==========================================//
                    //начинается ход за черных
                    if (board->currentMove == "Black") {

                        std::cout << "Отрабатывает минимакс" << std::endl;
                        //лучшая оценка
                        int bestScore = -99999;
                        int minEval = 99999;

                        //лучшего хода
                        CellStep bestStep;
                        int indexCurCell = -1;
                        if (board->currentCell && board->currentCell->type == "Black" && board->getValidEatenStep().size()) {
                            std::vector<CellStep> steps = board->getValidEatenStep();
                            for (int step = 0; step < steps.size(); step++) {
                                //создаем виртуальную доску с выбранным ходом
                                Board* newBoard = new Board(board->checkersCell, board->currentCell, board->currentMove, steps[step], board->sdl);
                                int newValue = minMaxAlgorithm(newBoard, 1);
                                std::cout << newValue << " -- ";

                                if (board->currentCell->type == "Black") {
                                    if (newValue > bestScore) {
                                        bestScore = newValue;
                                        bestStep = steps[step];
                                        indexCurCell = board->currentCell->y * 8 + board->currentCell->x;
                                    }
                                }
                                else if (board->currentCell->type == "White") {
                                    if (newValue < minEval) {
                                        minEval = newValue;
                                        bestStep = steps[step];
                                        indexCurCell = board->currentCell->y * 8 + board->currentCell->x;
                                    }
                                }

                                //if (newValue > bestScore) {
                                //    //запоминаем ход и текущую точку при этом ходе
                                //    bestScore = newValue;
                                //    bestStep = steps[step];
                                //    indexCurCell = board->currentCell->y * 8 + board->currentCell->x;
                                //}
                                newBoard->~Board();
                                delete newBoard;
                            }
                        }

                        else {
                            for (int cellInd = 0; cellInd < board->checkersCell.size(); cellInd++) {
                                //выбираем только черную пешку
                                if (board->checkersCell[cellInd]->type != "Black") {
                                    continue;
                                }

                                //выбираем пешку как текущую и ищем доступные ходы 
                                board->currentCell = board->checkersCell[cellInd];
                                std::vector<CellStep> steps = board->getAllValidSteps();
                                for (int step = 0; step < steps.size(); step++) {
                                    //создаем виртуальную доску с выбранным ходом
                                    Board* newBoard = new Board(board->checkersCell, board->currentCell, board->currentMove, steps[step], board->sdl);
                                    int newValue = minMaxAlgorithm(newBoard, 1);
                                    std::cout << newValue << " -- ";
                                    if (newValue > bestScore) {
                                        //запоминаем ход и текущую точку при этом ходе
                                        bestScore = newValue;
                                        bestStep = steps[step];
                                        indexCurCell = cellInd;
                                        newBoard->printBoard();
                                        std::cout << "SCORE: " << evaluation_function(newBoard) << std::endl;;
                                    }
                                    newBoard->~Board();
                                    delete newBoard;
                                }
                            }
                        }

                        //если есть лучший ход
                        if (bestStep.x != -1 && bestStep.y != -1 && indexCurCell != -1) {
                            board->currentCell = board->checkersCell[indexCurCell];
                            board->switchCheckers(board->currentCell, board->checkersCell[bestStep.y * 8 + bestStep.x]);
                            board->currentCell = board->checkersCell[bestStep.y * 8 + bestStep.x];
                            if (bestStep.eatenCell) {
                                bestStep.eatenCell->type = "Empty";
                                bestStep.eatenCell->isQueen = false;
                                if (!board->getValidEatenStep().size()) {
                                    board->switchColor();
                                    board->currentCell = nullptr;
                                }
                            }
                            else {
                                board->switchColor();
                                board->currentCell = nullptr;
                            }
                            
                        }
                        else {
                            std::cout << "Черным некуда ходить, конец игры" << std::endl;
                        }

                        std::cout << "Минимакс отработал" << std::endl;
                    }
                    //==========================================//
                }
            }
        }
    }
    catch (...) {
        std::cout << "Error in main: "<< std::endl;
    }

    return 0;
}
