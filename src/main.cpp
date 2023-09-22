/*
	https://gitlab.freedesktop.org/poppler/poppler/-/blob/master/cpp/tests/poppler-dump.cpp
	https://poppler.freedesktop.org/api/cpp/annotated.html
*/

#include <iostream>
#include <iomanip>

#include <json/json.h>

#include <poppler-document.h>
#include <poppler-font.h>
#include <poppler-page.h>

// Debug
static std::string debug_orientation(poppler::page::orientation_enum o)
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

static void debug_text_box(const poppler::text_box& text_box)
{
	std::string text = text_box.text().to_latin1();
	poppler::rectf bbox = text_box.bbox();
	
	std::cout << "[" << text << "] @ ";
	std::cout << "( x=" << bbox.x() << " y=" << bbox.y() << " w=" << bbox.width() << " h=" << bbox.height() << " )";
	
	if (text_box.has_font_info())
	{
		double font_size = text_box.get_font_size();
		int wmode = text_box.get_wmode();
		std::string font_name = text_box.get_font_name();
		
		std::cout << "( fontname=" << font_name << " fontsize=" << font_size << " wmode=" << wmode << " )";
	}
	std::cout << std::endl;
}

static void debug_page(poppler::page* page)
{
	std::vector<poppler::text_box> text_list = page->text_list(0);

	// Draw page information
	if (page)
	{
		std::cout << std::setw(10) << "Rect"        << ": " << page->page_rect() << std::endl;
		std::cout << std::setw(10) << "Label"       << ": " << page->label().to_latin1() << std::endl;
		std::cout << std::setw(10) << "Duration"    << ": " << page->duration() << std::endl;
		std::cout << std::setw(10) << "Orientation" << ": " << debug_orientation(page->orientation()) << std::endl;
	}
	else
	{
		std::cout << std::setw(10) << "Broken Page. Could not be parsed" << std::endl;
	}
	std::cout << std::endl;


	// Draw page content
	std::cout << "---" << std::endl;
	for (const poppler::text_box& text_box : text_list)
	{
		poppler::rectf bbox = text_box.bbox();
		poppler::ustring ustr = text_box.text();
		
		debug_text_box(text_box);
	}
	std::cout << "---" << std::endl;
}

static void debug_document(poppler::document* document)
{
	const int pages = document->pages();
	for (int i = 0; i < pages; ++i)
	{
		std::unique_ptr<poppler::page> page(document->create_page(i));
		
		std::cout << "Page " << (i + 1) << "/" << pages << ":" << std::endl;
		
		debug_page(page.get());
	}
}

struct TextBoxInfo
{
	poppler::text_box* from;
	poppler::text_box* to;
	poppler::text_box* overview;
	poppler::text_box* table;
	poppler::text_box* total;
	
	bool foundAll()
	{
		return (
			from != nullptr &&
			to != nullptr && 
			overview != nullptr && 
			table != nullptr && 
			total != nullptr 
		);
	}
	
	bool isFrom(const poppler::text_box& text_box) const
	{
		return text_box.text().to_latin1() == "From:";
	}
	
	bool isInsideFrom(const poppler::text_box& text_box) const
	{
		poppler::rectf bbox = text_box.bbox();
		
		return (
			bbox.y() >  from->bbox().y()    && // y >= from.y
			bbox.y() <  to->bbox().y()      && // y <  to.y
			bbox.x() <  overview->bbox().x()   // x <  overview.x
		);
	}
	
	bool isTo(const poppler::text_box& text_box) const
	{
		return text_box.text().to_latin1() == "To:";
	}
	
	bool isInsideTo(const poppler::text_box& text_box) const
	{
		poppler::rectf bbox = text_box.bbox();
		
		return (
			bbox.y() >  to->bbox().y()       &&    // y >= to.y
			bbox.y() <  table->bbox().y()    &&    // y <  table.y
			bbox.x() <  overview->bbox().x() - 150 // x <  overview.x
		);
	}
	
	bool isOverview(const poppler::text_box& text_box) const
	{
		poppler::rectf bbox = text_box.bbox();
		
		return (
			text_box.text().to_latin1() == "Invoice" &&
			bbox.x() < 350 &&
			bbox.x() > 345 &&
			bbox.y() < 140
		);
	}
	
	bool isInsideOverview(const poppler::text_box& text_box) const
	{
		poppler::rectf bbox = text_box.bbox();
		
		return (
			bbox.y() >=  overview->bbox().y() && // y >= overview.y
			bbox.y() <   table->bbox().y()    && // y <  table.y
			bbox.x() >=  overview->bbox().x()    // x >= overview.x
		);
	}
	
	bool isTable(const poppler::text_box& text_box) const
	{
		return (
			text_box.text().to_latin1() == "Hrs/Qty"
		);
	}
	
	bool isInsideTable(const poppler::text_box& text_box) const
	{
		poppler::rectf bbox = text_box.bbox();
		
		return (
			bbox.y() >= table->bbox().y() && // y >= table.y
			bbox.y() <  total->bbox().y()    // y <  total.y
		);
	}
	
	bool isTotal(const poppler::text_box& text_box) const
	{
		return (
			text_box.text().to_latin1() == "Sub"
		);
	}
	
	bool isInsideTotal(const poppler::text_box& text_box) const
	{
		poppler::rectf bbox = text_box.bbox();
		
		return (
			bbox.x() >= total->bbox().x() &&    // x >= total.x
			bbox.y() >= total->bbox().y() &&    // y >= total.y
			bbox.y() <  total->bbox().y() + 200 // y <  total.y
		);
	}
};

bool extractTextBoxInfo(const std::vector<poppler::text_box>& text_box_list, TextBoxInfo& text_box_info)
{
	for (const poppler::text_box& text_box : text_box_list)
	{
		if(text_box_info.isFrom(text_box))
			text_box_info.from = const_cast<poppler::text_box*>(&text_box);
		
		else if(text_box_info.isTo(text_box))
			text_box_info.to = const_cast<poppler::text_box*>(&text_box);
		
		else if(text_box_info.isOverview(text_box))
			text_box_info.overview = const_cast<poppler::text_box*>(&text_box);
		
		else if(text_box_info.isTable(text_box))
			text_box_info.table = const_cast<poppler::text_box*>(&text_box);
		
		else if(text_box_info.isTotal(text_box))
			text_box_info.total = const_cast<poppler::text_box*>(&text_box);
	}
	
	// Debug
	//std::cout << "from:     " << text_box_info.to       << std::endl;
	//std::cout << "to:       " << text_box_info.from     << std::endl;
	//std::cout << "overview: " << text_box_info.overview << std::endl;
	//std::cout << "table:    " << text_box_info.table    << std::endl;
	//std::cout << "total:    " << text_box_info.total    << std::endl;
	
	return text_box_info.foundAll();
}

typedef bool (TextBoxInfo::*TextBoxInfoIsFunc)(const poppler::text_box&) const;

std::vector<std::string> convertToString(const std::vector<poppler::text_box>& text_box_list, const TextBoxInfo& text_box_info,
		TextBoxInfoIsFunc is_func
)
{
	double last_y = 0.0;
	std::vector<std::string> results;
		
	for (const poppler::text_box& text_box : text_box_list)
	{	
		if((const_cast<TextBoxInfo*>(&text_box_info)->*(is_func))(text_box))
		{
			std::string text = text_box.text().to_latin1();
			double y = text_box.bbox().y();
				
			if(last_y == y)
			{
				results.back() += " " + text;
			}
			else
			{
				results.push_back(text);
				last_y = y;
			}
		}
		
		//debug_text_box(text_box);
	}
	
	return results;
}

void extractFrom(const std::vector<poppler::text_box>& text_box_list, const TextBoxInfo& text_box_info, Json::Value& json_results)
{
	Json::Value json_from;
	std::vector<std::string> str_results;
	
	// Convert to string
	str_results = convertToString(text_box_list, text_box_info, &TextBoxInfo::isInsideFrom);
	
	// Debug
	//for(std::string str_result : str_results)
	//	std::cout << str_result << std::endl;
	
	// Create json from
	json_from["name"] = str_results[0];
	json_from["street"] = str_results[2] + " " + str_results[1];
	json_from["postcode"] = str_results[3];
	json_from["email"] = str_results[4];
	
	// Append json results
	json_results["from"] = json_from;
}

void extractTo(const std::vector<poppler::text_box>& text_box_list, const TextBoxInfo& text_box_info, Json::Value& json_results)
{
	std::vector<std::string> str_results;
	Json::Value json_to;
	
	str_results = convertToString(text_box_list, text_box_info, &TextBoxInfo::isInsideTo);
	
	// Debug
	//for(std::string str_result : str_results)
	//	std::cout << str_result << std::endl;

	// Create json from
	json_to["name"] = str_results[0];
	json_to["street"] = str_results[1];
	json_to["postcode"] = str_results[2];
	json_to["email"] = str_results[3];
	
	// Append json results
	json_results["to"] = json_to;
}

void extractOverview(const std::vector<poppler::text_box>& text_box_list, const TextBoxInfo& text_box_info, Json::Value& json_results)
{
	std::vector<std::string> str_results;
	Json::Value json_overview;
	
	str_results = convertToString(text_box_list, text_box_info, &TextBoxInfo::isInsideOverview);
	
	// Debug
	//for(std::string str_result : str_results)
	//	std::cout << str_result << std::endl;
	
	json_overview["invoice_number"] = str_results[0].substr(15);
	json_overview["order_number"] = str_results[1].substr(13);
	json_overview["invoice_date"] = str_results[2].substr(13);
	json_overview["due_date"] = str_results[3].substr(9);
	json_overview["total_due"] = str_results[4].substr(10);
	
	json_results["overview"] = json_overview;
}

void extractTable(const std::vector<poppler::text_box>& text_box_list, const TextBoxInfo& text_box_info, Json::Value& json_results)
{
	std::vector<std::string> str_results;
	Json::Value json_overview;
	
	str_results = convertToString(text_box_list, text_box_info, &TextBoxInfo::isInsideTable);
	
	// Debug
	//for(std::string str_result : str_results)
	//	std::cout << str_result << std::endl;
}

void extractTotal(const std::vector<poppler::text_box>& text_box_list, const TextBoxInfo& text_box_info, Json::Value& json_results)
{
	std::vector<std::string> str_results;
	Json::Value json_total;
	
	str_results = convertToString(text_box_list, text_box_info, &TextBoxInfo::isInsideTotal);
	
	// Debug
	//for(std::string str_result : str_results)
	//	std::cout << str_result << std::endl;
	
	json_total["sub_total"] = str_results[0].substr(10);
	json_total["tax"] = str_results[1].substr(4);
	json_total["total"] = str_results[2].substr(6);
	
	json_results["total"] = json_total;
}

void extract(poppler::document* document, Json::Value& json_results)
{
	const int pages = document->pages();
	for (int i = 0; i < pages; ++i)
	{
		std::unique_ptr<poppler::page> page(document->create_page(i));
		
		std::vector<poppler::text_box> text_box_list = page->text_list(0);
		TextBoxInfo text_box_info;
		
		// Extract the important elements in the documents
		if(extractTextBoxInfo(text_box_list, text_box_info))
		{
			// Extract data
			extractFrom(text_box_list, text_box_info, json_results);
			extractTo(text_box_list, text_box_info, json_results);
			extractOverview(text_box_list, text_box_info, json_results);
			//extractTable(text_box_list, text_box_info, json_results);
			extractTotal(text_box_list, text_box_info, json_results);
		}
	}
}

bool open_pdf(std::unique_ptr<poppler::document>& document, const std::string& file_name)
{
	poppler::document* doc;
	
	// Open pdf file
	doc = poppler::document::load_from_file(file_name);
	
	// Check document
	if (!doc || doc->is_locked())
	{
		return false;
	}

	// Transfer ownership
	document.reset(doc);
	
	return true;
}

int main()
{
	std::string file_name = "../data/wordpress-pdf-invoice-plugin-sample.pdf";
	std::unique_ptr<poppler::document> document;
	Json::Value json_results;
	
	if(!open_pdf(document, file_name))
	{
		std::cout << "[Error] Can't Open pdf file." << std::endl;
		return 0;
	}
	
	//debug_document(document.get());
	
	// Extract information to json
	extract(document.get(), json_results);
	
	// Create a JSON writer
	Json::StreamWriterBuilder writer;
	std::string jsonString = Json::writeString(writer, json_results);
	
	std::cout << jsonString << std::endl;

	return 0;
}

