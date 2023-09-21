/*
	https://gitlab.freedesktop.org/poppler/poppler/-/blob/master/cpp/tests/poppler-dump.cpp
	https://poppler.freedesktop.org/api/cpp/annotated.html
*/

#include <iostream>
#include <iomanip>

#include <poppler-document.h>
#include <poppler-font.h>
#include <poppler-page.h>

static std::string out_page_orientation(poppler::page::orientation_enum o)
{
    switch (o)
	{
		case poppler::page::landscape:
			return "landscape (90)";
		case poppler::page::portrait:
			return "portrait (0)";
		case poppler::page::seascape:
			return "seascape (270)";
		case poppler::page::upside_down:
			return "upside_downs (180)";
    };
	
    return "<unknown orientation>";
}

static void print_page(poppler::page* p)
{
	// Draw page information
    if (p)
	{
        std::cout << std::setw(30) << "Rect"        << ": " << p->page_rect() << std::endl;
        std::cout << std::setw(30) << "Label"       << ": " << p->label().to_latin1() << std::endl;
        std::cout << std::setw(30) << "Duration"    << ": " << p->duration() << std::endl;
        std::cout << std::setw(30) << "Orientation" << ": " << out_page_orientation(p->orientation()) << std::endl;
    }
	else
	{
        std::cout << std::setw(30) << "Broken Page. Could not be parsed" << std::endl;
    }
    std::cout << std::endl;
	
	// Draw page content
	auto text_list = p->text_list(0);
    std::cout << "---" << std::endl;
    for (const poppler::text_box &text : text_list)
	{
        poppler::rectf bbox = text.bbox();
        poppler::ustring ustr = text.text();
        int wmode = text.get_wmode();
        double font_size = text.get_font_size();
        std::string font_name = text.get_font_name();
		
        std::cout << "[" << ustr.to_latin1() << "] @ ";
        std::cout << "( x=" << bbox.x() << " y=" << bbox.y() << " w=" << bbox.width() << " h=" << bbox.height() << " )";
        if (text.has_font_info())
		{
            std::cout << "( fontname=" << font_name << " fontsize=" << font_size << " wmode=" << wmode << " )";
        }
        std::cout << std::endl;
    }
    std::cout << "---" << std::endl;
}


int main()
{
	std::string file_name = "../data/wordpress-pdf-invoice-plugin-sample.pdf";

	std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(file_name));

	if (!doc.get())
	{
		std::cout << "loading error" << std::endl;
		return 0;
	}

	if (doc->is_locked())
	{
		std::cout << "encrypted document" << std::endl;
		return 0;
	}

	const int pages = doc->pages();
	for (int i = 0; i < pages; ++i)
	{
		std::cout << "Page " << (i + 1) << "/" << pages << ":" << std::endl;
		
		std::unique_ptr<poppler::page> p(doc->create_page(i));
		print_page(p.get());
	}

	return 0;
}