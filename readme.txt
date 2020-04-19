используетс€ vcpkg
при сборке vcpkg возможно сообщение
/include/vcpkg/packagespec.h:22:9: note: Сvcpkg::PackageSpec::PackageSpec() noexceptТ is implicitly deleted because its exception-specification does not match the implicit exception-specification СТ
         PackageSpec() noexcept = default;
в файле /include/vcpkg/packagespec.h удалить  noexept у классов Package и PackageSpec

vcpkg install cpprestsdk // собирает zlib, boost, openssl, cpprest, bzip2, liblzma, libiconv, zstd
vcpkg install boost
vcpkg install boost-interprocess
vcpkg install boost-gil

сборка под win7
	cd build
	cmake .. -T v140 -DCMAKE_SYSTEM_VERSION=8.1 -DCMAKE_TOOLCHAIN_FILE=D:\lib\vcpkg\scripts\buildsystems\vcpkg.cmake
	cmake --build . --config Release -j 4

сборка под linux
	cd build
	cmake .. /opt/vcpkg/scripts/buildsystems/vcpkg.cmake
	cmake --build . --config Release -j 4

зависимости
	cpprestsdk	2.10 
	boost		1.72 
	zlib
	libpng
	libjpeg


#define MAX_BOZORTH_MINUTIAE		200

typedef struct {
	int nrows;
	int xcol[MAX_BOZORTH_MINUTIAE];
	int ycol[MAX_BOZORTH_MINUTIAE];
	int thetacol[MAX_BOZORTH_MINUTIAE];
	int quality[MAX_BOZORTH_MINUTIAE];
} xyt_struct; 
 

// сравниваем два образца
extern "C" int matchSegments(struct fp_img* image1, struct fp_img* image2);

// сравниваем из образец с шаблоном
extern "C" int matchSegmentsImgTemplate(struct fp_img* image1, unsigned char* tmpl);

//сравниваем два шаблона
extern "C" int matchSegmentsTemplate(unsigned char* tmpl1, int cx1, int cy1,  unsigned char* tmpl2,int cx2,int cy2);


//10.6.46.130 - DB
// postgres/g6nsgWG2Xk
CREATE OR REPLACE FUNCTION insert_data(real[], integer, integer, text, text, text, text) RETURNS text AS '/home/baltaev/abis/abisfunc.so', 'insert_data' LANGUAGE ...
// 1 - кол-во частей вектор
// 2 - им€ базы
// 3 - им€ таблицы параметров алгоритма поиска
// 4 - им€ таблицы с индексами (соседи по графу)
// 5 - им€ таблицы с векторами (шаблонами)
select init(8, 'dbABIS', 'test_params', 'test_index', 'test_table', 800, 32, 300);

// вставка шаблона
// 1 - вектор
// 2 - его ид
// 3 - длина части (длина вектора / на кол-во частей)
// 4 - им€ базы
// 5 - им€ таблицы параметров алгоритма поиска
// 6 - им€ таблицы с индексами (соседи по графу)
// 7 - им€ таблицы с векторами (шаблонами)
select insert_data(ARRAY[0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5], 1, 2, 'dbABIS', 'test_params', 'test_index', 'test_table');

// удаление шаблона
// 1 - кол-во частей вектор
// 2 - его ид
// 3 - им€ таблицы параметров алгоритма поиска
// 4 - им€ таблицы с индексами (соседи по графу)
// 5 - им€ таблицы с векторами (шаблонами)
select delete_data(8, 3, 'dbABIS', 'test_params', 'test_index', 'test_table');

// поиск шаблона по базе
// 1 - вектор
// 2 - длина части (длина вектора / на кол-во частей)
// 3 - кол-во возвращаемых соседей
// 4 - им€ таблицы параметров алгоритма поиска
// 5 - им€ таблицы с индексами (соседи по графу)
// 6 - им€ таблицы с векторами (шаблонами)
select search_data(ARRAY[0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5], 2, 3, 'dbABIS', 'test_params', 'test_index', 'test_table'); 