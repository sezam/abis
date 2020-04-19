������������ vcpkg
��� ������ vcpkg �������� ���������
/include/vcpkg/packagespec.h:22:9: note: �vcpkg::PackageSpec::PackageSpec() noexcept� is implicitly deleted because its exception-specification does not match the implicit exception-specification ��
         PackageSpec() noexcept = default;
� ����� /include/vcpkg/packagespec.h �������  noexept � ������� Package � PackageSpec

vcpkg install cpprestsdk // �������� zlib, boost, openssl, cpprest, bzip2, liblzma, libiconv, zstd
vcpkg install boost
vcpkg install boost-interprocess
vcpkg install boost-gil

������ ��� win7
	cd build
	cmake .. -T v140 -DCMAKE_SYSTEM_VERSION=8.1 -DCMAKE_TOOLCHAIN_FILE=D:\lib\vcpkg\scripts\buildsystems\vcpkg.cmake
	cmake --build . --config Release -j 4

������ ��� linux
	cd build
	cmake .. /opt/vcpkg/scripts/buildsystems/vcpkg.cmake
	cmake --build . --config Release -j 4

�����������
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
 

// ���������� ��� �������
extern "C" int matchSegments(struct fp_img* image1, struct fp_img* image2);

// ���������� �� ������� � ��������
extern "C" int matchSegmentsImgTemplate(struct fp_img* image1, unsigned char* tmpl);

//���������� ��� �������
extern "C" int matchSegmentsTemplate(unsigned char* tmpl1, int cx1, int cy1,  unsigned char* tmpl2,int cx2,int cy2);


//10.6.46.130 - DB
// postgres/g6nsgWG2Xk
CREATE OR REPLACE FUNCTION insert_data(real[], integer, integer, text, text, text, text) RETURNS text AS '/home/baltaev/abis/abisfunc.so', 'insert_data' LANGUAGE ...
// 1 - ���-�� ������ ������
// 2 - ��� ����
// 3 - ��� ������� ���������� ��������� ������
// 4 - ��� ������� � ��������� (������ �� �����)
// 5 - ��� ������� � ��������� (���������)
select init(8, 'dbABIS', 'test_params', 'test_index', 'test_table', 800, 32, 300);

// ������� �������
// 1 - ������
// 2 - ��� ��
// 3 - ����� ����� (����� ������� / �� ���-�� ������)
// 4 - ��� ����
// 5 - ��� ������� ���������� ��������� ������
// 6 - ��� ������� � ��������� (������ �� �����)
// 7 - ��� ������� � ��������� (���������)
select insert_data(ARRAY[0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5], 1, 2, 'dbABIS', 'test_params', 'test_index', 'test_table');

// �������� �������
// 1 - ���-�� ������ ������
// 2 - ��� ��
// 3 - ��� ������� ���������� ��������� ������
// 4 - ��� ������� � ��������� (������ �� �����)
// 5 - ��� ������� � ��������� (���������)
select delete_data(8, 3, 'dbABIS', 'test_params', 'test_index', 'test_table');

// ����� ������� �� ����
// 1 - ������
// 2 - ����� ����� (����� ������� / �� ���-�� ������)
// 3 - ���-�� ������������ �������
// 4 - ��� ������� ���������� ��������� ������
// 5 - ��� ������� � ��������� (������ �� �����)
// 6 - ��� ������� � ��������� (���������)
select search_data(ARRAY[0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.0, 0.1, 0.2, 0.3, 0.4, 0.5], 2, 3, 'dbABIS', 'test_params', 'test_index', 'test_table'); 