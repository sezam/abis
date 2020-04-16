#ifndef __BMP_H__
#define __BMP_H__

typedef struct{
    unsigned int width;
    unsigned int height;
    unsigned int *image;
}etna_bo;


/**
@struct palette
@brief Структура палитры.
*/
struct palette {
    unsigned char B;    ///< компонента синего цвета
    unsigned char G;    ///< компонента зеленого цвета
    unsigned char R;    ///< компонента красного цвета
    unsigned char fil;  ///< зарезервиравано и не используется
};

/**
@struct bmpheader
@brief Структура заголовка .bmp файла.
*/
#pragma pack(push,1)
struct bmpheader {
    char id1;                       ///< Первая часть сигнатуры "B"
    char id2;                       ///< Вторая часть сигнатуры "M"
    int FileSize;                   ///< размер файла
    int Reserved;                   ///< зарезервировано
    int DataOffset;                 ///< смещение данных для постоения изображения относительно начала файла
    int HeaderSize;                 ///< размер заголовка
    int Width;                      ///< ширина изображения
    int Height;                     ///< высота изображения
    unsigned short Planes;          ///< Число плоскостей, должно быть 1
    unsigned short BitsPerPixel;    ///< количество бит на обну точку изображения
    int Compression;                ///< тип сжатия
    int DataSize;                   ///< размер данных
    int HResolution;                ///< разрешение по горизонтали, пиксел/м
    int VResolution;                ///< разрешение по вертикали, пиксел/м
    int Colors;                     ///< Количество используемых цветов
    int ImColors;                   ///< Количество "важных" цветов.
    palette pal[256];               ///< Палитра, для менее, чем 8-битного изображения.
};
#pragma pack(pop)

/**
@class tBitmap
@brief Структура палитры.
*/
class tBitmap
{
	private:
		unsigned int left;                   ///< координата X начала изображения (левый верхний угол)
		unsigned int top;                    ///< координата Y начала изображения (левый верхний угол)
		unsigned int wid;                    ///< ширина изображения в точках
		unsigned int hei;                    ///< высота изображения в точках
		unsigned char al;                    ///< прозрачность в процентах
		unsigned int *pimage;                ///< указатель на данные изображения
        unsigned int colorKey;               ///< Цветовой ключ (данный цвет не отображается при выводе картинки)
        bool colorKeyPresent;                ///< флаг присутствия цветового ключа
        etna_bo *image;                      ///< указатель на поверхность, содержащая изображение
		bool vis;                            ///< разрешение на показ изображения. 0 - изображение не показывается, 1 - показать изображение.
        unsigned char *grayImage;            ///< Изображение в оттенках серого
	public:
        tBitmap();
        /**
        *@fn void setPosition(int x, int y);
        *@brief Функция устанавливает координаты верхнего левого угла для вывода изображения.
        *@param x координата по оси X.
        *@param y координата по оси Y.
        */
		void setPosition(int x, int y);
        /**
        *@fn void loadBitmap (char *name);
        *@brief Функция загружает bmp файл.
        *@param name имя файла для загрузки.
        */
		void loadBitmap (char *name);

        /**
        * Загружает bmp из буфера
        */
        int loadBitmap(unsigned char* image_data, const unsigned int image_len);
        /**
        *@fn void getData(unsigned short* width, unsigned short* height, unsigned short data[]);
        *@brief Функция получает изображение для дальнейшей его обработки.
        *@param width Ширина.
        *@param height Высота.
        *@param data данные изображения
        */
		void getData(unsigned int* width, unsigned int* height, unsigned int data[]);
        /**
        *@fn void visible(bool f);
        *@brief Функция показывает или убирает изображение.
        *@param f 0 - убрать изображение с экрана, 1 - показать изображение.
        */
		void visible(bool f);
        /**
        *@fn void setalpha(char a);
        *@brief Функция устанавливает уровень прозрачности.
        *@param a прозрачность в процентах.
        */
		void setalpha(unsigned char a);
        /**
        *@fn int getVisible();
        *@brief возвращает состояние видимости картинки.
        *@return 0-невидима 1-показывается.
        */
        int getVisible();
        /**
        *@fn int getWidth();
        *@brief возвращает длину картинки в точках.
        *@return длина.
        */
        int getWidth();
        /**
        *@fn int getHeight();
        *@brief возвращает высоту картинки в точках.
        *@return высота.
        */
        int getHeight();
        /**
        *@fn unsigned int *getImagePointer();
        *@brief возвращает указатель на изображение картинки.
        *@return указатель.
        */
        unsigned int *getImagePointer();
        /**
        *@fn void clearVisible();
        *@brief сбрасывает свойство видимости объекта
        */
        void clearVisible();
        /**
        *@fn void setColorKey(unsigned int col);
        *@brief Устанавливает цветовой ключ
        *@param col цвет.
        */
        void setColorKey(unsigned int col);
        /**
        *@fn void refrashImage();
        *@brief добавляет изображение в очередь для отображения
        */
        void refrashImage();
        /**
        *@fn void destroyImage();
        *@brief Функция удаляет изображение.
        */
		void destroyImage();
        unsigned int RGBToColor(unsigned char alpha, unsigned char r, unsigned char g, unsigned char b);
        void colorToRGB(unsigned int color, unsigned char *alpha, unsigned char *r, unsigned char *g, unsigned char *b);
        void makeGrayImage();
        unsigned char *getGrayImagePointer();
        void imgMean(int size);
        ~tBitmap();
};


#endif
