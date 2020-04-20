#include "AbisRest.h"
#include "biocard.h"
#include "compare.h"
#include "echotest.h"
#include "extract.h"
#include "verify.h"
#include "search.h"
#include "ebsclient.h"
#include "restutils.h"

void rest_server()
{
	uri_builder endpointBuilder;
	endpointBuilder.set_scheme(U("http"));
	endpointBuilder.set_port(10101);
#ifdef _WIN32
    endpointBuilder.set_host(U("*"));
#else
    endpointBuilder.set_host(U("0.0.0.0"));
#endif // _WIN32


	try
	{
        endpointBuilder.set_path(U("echo"));
        http_listener echo_listener = register_echo(endpointBuilder.to_uri());

        endpointBuilder.set_path(U("extract"));
        http_listener extract_listener = register_extract(endpointBuilder.to_uri());

        endpointBuilder.set_path(U("compare"));
        http_listener compare_listener = register_compare(endpointBuilder.to_uri());

        endpointBuilder.set_path(U("biocard"));
        http_listener biocard_listener = register_biocard(endpointBuilder.to_uri());

        endpointBuilder.set_path(U("verify"));
        http_listener verify_listener = register_verify(endpointBuilder.to_uri());

        endpointBuilder.set_path(U("search"));
        http_listener search_listener = register_search(endpointBuilder.to_uri());

		while (true);

		echo_listener.close().wait();
		extract_listener.close().wait();
		compare_listener.close().wait();
		biocard_listener.close().wait();
		verify_listener.close().wait();
		search_listener.close().wait();
	}
	catch (exception const& e)
	{
		cout << e.what() << endl;
	}
}

void test_ebs_client()
{
	FILE* file;
	unsigned char* buffer;
	unsigned long fileLen;

	file = fopen("c:\\temp\\1\\1.png", "rb");
	if (!file)
	{
		fprintf(stderr, "can't open file %s", "c:\\temp\\1\\1.png");
		exit(1);
	}

	fseek(file, 0, SEEK_END);
	fileLen = ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer = (unsigned char*)malloc(fileLen + 1);

	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
		fclose(file);
		exit(1);
	}

	fread(buffer, fileLen, 1, file);
	fclose(file);

	float biotemplate[FACE_TEMPLATE_SIZE];
	int res = get_face_template(buffer, fileLen + 1, biotemplate, sizeof(biotemplate));
	printf("result is %d \n", res);

	free(buffer);
}

int main()
{
	//test_ebs_client();
	rest_server();

	return 0;
}