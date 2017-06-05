// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "../libs0.hpp"
#include "gtest/gtest.h"
#include "../utils/time_utils.hpp"

using namespace time_utils;

// TODO write unit test that meet "Leap second" exception

TEST(time_utils, check_readable_time_length) {

	std::string formated = time_t_to_readable(gen_exact_date(1993,6,1,1,1,1));
	EXPECT_TRUE(formated.length() == 25);

	formated = time_t_to_readable(gen_exact_date(1983,12,30,2,2,2));
	EXPECT_TRUE(formated.length() == 25);

	formated = time_t_to_readable(gen_exact_date(2923,10,20,3,3,3));
	EXPECT_TRUE(formated.length() == 25);

}

TEST(time_utils, check_readable_time_regex) {

	//example match: 2017-01-02T09:10:53+00:00
	const std::string year_regexp(R"((?:[1-9]\d{3})");
	const std::string month_regexp(R"((?:(?:0[1-9]|1[0-2]))");
	const std::string day_regexp(R"((?:0[1-9]|1\d|2[0-8])|(?:0[13-9]|1[0-2])-(?:29|30)|(?:0[13578]|1[02])-31)|(?:[1-9]\d(?:0[48]|[2468][048]|[13579][26])|(?:[2468][048]|[13579][26])00)-02-29))");
	const std::string hour(R"((?:[01]\d|2[0-3]):[0-5]\d:[0-5]\d)");
	const std::string timeshift(R"((?:Z|[+-][01]\d:[0-5]\d))");
	std::regex re("^" + year_regexp + "-" + month_regexp + "-" + day_regexp + "T" + hour + timeshift + "$");
	std::smatch result;


	std::string formated = time_t_to_readable(gen_exact_date(1993,6,1,1,1,1));
	_mark("time: " << formated << ", formated_length: " << formated.length());
	EXPECT_TRUE(std::regex_match(formated, result, re));

	formated = time_t_to_readable(gen_exact_date(1983,12,30,2,2,2));
	EXPECT_TRUE(std::regex_match(formated, result, re));
	_mark("time: " << formated << ", formated_length: " << formated.length());

	formated = time_t_to_readable(gen_exact_date(2923,10,20,3,3,3));
	EXPECT_TRUE(std::regex_match(formated, result, re));
	_mark("time: " << formated << ", formated_length: " << formated.length());

	formated = time_t_to_readable(gen_exact_date(2017,4,20,5,30,15));
	EXPECT_TRUE(std::regex_match(formated, result, re));
	_mark("time: " << formated << ", formated_length: " << formated.length());
}

TEST(time_utils, t_timepoint_overload) {
	t_timepoint now_time = t_timepoint::clock::now();
	std::string formated = timepoint_to_readable(now_time);
	EXPECT_TRUE(formated.length() == 25);
}

// Getting exact time with zone utc offset on mingw is not supported for now.
// The reason is that boost::date_time local_time() getting time from win setting not from TZ variable.
// There is solution to use boost::local_time::tz_database and load date_time_zonespec.csv,
// but this solution is not satisfactory.
#ifndef ANTINET_windows
TEST(time_utils, exact_data_check) {

	std::string formated = time_t_to_readable(gen_exact_date(2017,10,21,12,1,3), "Europe/Warsaw");
	_mark("time Europe/Warsaw: " << formated << ", formated_length: " << formated.length());
	EXPECT_EQ(formated, "2017-10-21T12:01:03+02:00");

	formated = time_t_to_readable(gen_exact_date(2017,10,1,0,0,0), "Asia/Pyongyang");
	_mark("time Asia/Pyongyang: " << formated << ", formated_length: " << formated.length());
	EXPECT_EQ(formated, "2017-10-01T00:00:00+08:30");

	formated = time_t_to_readable(gen_exact_date(2022,12,15,15,0,0), "Pacific/Galapagos");
	_mark("time Pacific/Galapagos: " << formated << ", formated_length: " << formated.length());
	EXPECT_EQ(formated, "2022-12-15T15:00:00-06:00");
}
#endif

TEST(time_utils, daylight_saving) {

	std::string formated = time_t_to_readable(gen_exact_date(2016,12,6,12,0,0,1), "Europe/Warsaw");
	std::string time_to_match = "2016-12-06T12:00:00";
	//_mark("Testing zone: " << zone << " formated: " << formated);
	EXPECT_NE(formated.find(time_to_match),std::string::npos)
			<< "Generated time ["<< formated
			<< "] not match expected ["<< time_to_match <<"]";
}

TEST(time_utils, default_tz) {

	unsetenv("TZ");
	std::string formated = time_t_to_readable(gen_exact_date(2019,11,21,11,10,30));
	EXPECT_TRUE(formated.find("2019-11-21T11:10:30") != std::string::npos);
}

TEST(time_utils, all_zones_envirnoment_test) {
	std::vector<std::string> all_zones{"Europe/Andorra", "Asia/Dubai", "Asia/Kabul", "America/Antigua", "America/Anguilla",
	                         "Europe/Tirane", "Asia/Yerevan", "Africa/Luanda", "Antarctica/McMurdo", "Antarctica/Casey",
	                         "Antarctica/Davis", "Antarctica/DumontDUrville", "Antarctica/Mawson", "Antarctica/Palmer",
	                         "Antarctica/Rothera", "Antarctica/Syowa", "Antarctica/Troll", "Antarctica/Vostok",
	                         "America/Argentina/Buenos_Aires", "America/Argentina/Cordoba", "America/Argentina/Salta",
	                         "America/Argentina/Jujuy", "America/Argentina/Tucuman", "America/Argentina/Catamarca",
	                         "America/Argentina/La_Rioja", "America/Argentina/San_Juan", "America/Argentina/Mendoza",
	                         "America/Argentina/San_Luis", "America/Argentina/Rio_Gallegos", "America/Argentina/Ushuaia",
	                         "Pacific/Pago_Pago", "Europe/Vienna", "Australia/Lord_Howe", "Antarctica/Macquarie",
	                         "Australia/Hobart", "Australia/Currie", "Australia/Melbourne", "Australia/Sydney",
	                         "Australia/Broken_Hill", "Australia/Brisbane", "Australia/Lindeman", "Australia/Adelaide",
	                         "Australia/Darwin", "Australia/Perth", "Australia/Eucla", "America/Aruba", "Europe/Mariehamn",
	                         "Asia/Baku", "Europe/Sarajevo", "America/Barbados", "Asia/Dhaka", "Europe/Brussels",
	                         "Africa/Ouagadougou", "Europe/Sofia", "Asia/Bahrain", "Africa/Bujumbura", "Africa/Porto-Novo",
	                         "America/St_Barthelemy", "Atlantic/Bermuda", "Asia/Brunei", "America/La_Paz", "America/Kralendijk",
	                         "America/Noronha", "America/Belem", "America/Fortaleza", "America/Recife", "America/Araguaina",
	                         "America/Maceio", "America/Bahia", "America/Sao_Paulo", "America/Campo_Grande", "America/Cuiaba",
	                         "America/Santarem", "America/Porto_Velho", "America/Boa_Vista", "America/Manaus",
	                         "America/Eirunepe", "America/Rio_Branco", "America/Nassau", "Asia/Thimphu", "Africa/Gaborone",
	                         "Europe/Minsk", "America/Belize", "America/St_Johns", "America/Halifax", "America/Glace_Bay",
	                         "America/Moncton", "America/Goose_Bay", "America/Blanc-Sablon", "America/Toronto",
	                         "America/Nipigon", "America/Thunder_Bay", "America/Iqaluit", "America/Pangnirtung",
	                         "America/Atikokan", "America/Winnipeg", "America/Rainy_River", "America/Resolute",
	                         "America/Rankin_Inlet", "America/Regina", "America/Swift_Current", "America/Edmonton",
	                         "America/Cambridge_Bay", "America/Yellowknife", "America/Inuvik", "America/Creston",
	                         "America/Dawson_Creek", "America/Fort_Nelson", "America/Vancouver", "America/Whitehorse",
	                         "America/Dawson", "Indian/Cocos", "Africa/Kinshasa", "Africa/Lubumbashi", "Africa/Bangui",
	                         "Africa/Brazzaville", "Europe/Zurich", "Africa/Abidjan", "Pacific/Rarotonga", "America/Santiago",
	                         "Pacific/Easter", "Africa/Douala", "Asia/Shanghai", "Asia/Urumqi", "America/Bogota",
	                         "America/Costa_Rica", "America/Havana", "Atlantic/Cape_Verde", "America/Curacao",
	                         "Indian/Christmas", "Asia/Nicosia", "Europe/Prague", "Europe/Berlin", "Europe/Busingen",
	                         "Africa/Djibouti", "Europe/Copenhagen", "America/Dominica", "America/Santo_Domingo",
	                         "Africa/Algiers", "America/Guayaquil", "Pacific/Galapagos", "Europe/Tallinn",
	                         "Africa/Cairo", "Africa/El_Aaiun", "Africa/Asmara", "Europe/Madrid", "Africa/Ceuta",
	                         "Atlantic/Canary", "Africa/Addis_Ababa", "Europe/Helsinki", "Pacific/Fiji", "Atlantic/Stanley",
	                         "Pacific/Chuuk", "Pacific/Pohnpei", "Pacific/Kosrae", "Atlantic/Faroe", "Europe/Paris",
	                         "Africa/Libreville", "Europe/London", "America/Grenada", "Asia/Tbilisi", "America/Cayenne",
	                         "Europe/Guernsey", "Africa/Accra", "Europe/Gibraltar", "America/Godthab", "America/Danmarkshavn",
	                         "America/Scoresbysund", "America/Thule", "Africa/Banjul", "Africa/Conakry", "America/Guadeloupe",
	                         "Africa/Malabo", "Europe/Athens", "Atlantic/South_Georgia", "America/Guatemala", "Pacific/Guam",
	                         "Africa/Bissau", "America/Guyana", "Asia/Hong_Kong", "America/Tegucigalpa", "Europe/Zagreb",
	                         "America/Port-au-Prince", "Europe/Budapest", "Asia/Jakarta", "Asia/Pontianak", "Asia/Makassar",
	                         "Asia/Jayapura", "Europe/Dublin", "Asia/Jerusalem", "Europe/Isle_of_Man", "Asia/Kolkata",
	                         "Indian/Chagos", "Asia/Baghdad", "Asia/Tehran", "Atlantic/Reykjavik", "Europe/Rome",
	                         "Europe/Jersey", "America/Jamaica", "Asia/Amman", "Asia/Tokyo", "Africa/Nairobi", "Asia/Bishkek",
	                         "Asia/Phnom_Penh", "Pacific/Tarawa", "Pacific/Enderbury", "Pacific/Kiritimati", "Indian/Comoro",
	                         "America/St_Kitts", "Asia/Pyongyang", "Asia/Seoul", "Asia/Kuwait", "America/Cayman", "Asia/Almaty",
	                         "Asia/Qyzylorda", "Asia/Aqtobe", "Asia/Aqtau", "Asia/Oral", "Asia/Vientiane", "Asia/Beirut",
	                         "America/St_Lucia", "Europe/Vaduz", "Asia/Colombo", "Africa/Monrovia", "Africa/Maseru",
	                         "Europe/Vilnius", "Europe/Luxembourg", "Europe/Riga", "Africa/Tripoli", "Africa/Casablanca",
	                         "Europe/Monaco", "Europe/Chisinau", "Europe/Podgorica", "America/Marigot", "Indian/Antananarivo",
	                         "Pacific/Majuro", "Pacific/Kwajalein", "Europe/Skopje", "Africa/Bamako", "Asia/Rangoon",
	                         "Asia/Ulaanbaatar", "Asia/Hovd", "Asia/Choibalsan", "Asia/Macau", "Pacific/Saipan",
	                         "America/Martinique", "Africa/Nouakchott", "America/Montserrat", "Europe/Malta",
	                         "Indian/Mauritius", "Indian/Maldives", "Africa/Blantyre", "America/Mexico_City", "America/Cancun",
	                         "America/Merida", "America/Monterrey", "America/Matamoros", "America/Mazatlan",
	                         "America/Chihuahua", "America/Ojinaga", "America/Hermosillo", "America/Tijuana",
	                         "America/Bahia_Banderas", "Asia/Kuala_Lumpur", "Asia/Kuching", "Africa/Maputo", "Africa/Windhoek",
	                         "Pacific/Noumea", "Africa/Niamey", "Pacific/Norfolk", "Africa/Lagos", "America/Managua",
	                         "Europe/Amsterdam", "Europe/Oslo", "Asia/Kathmandu", "Pacific/Nauru", "Pacific/Niue",
	                         "Pacific/Auckland", "Pacific/Chatham", "Asia/Muscat", "America/Panama", "America/Lima",
	                         "Pacific/Tahiti", "Pacific/Marquesas", "Pacific/Gambier", "Pacific/Port_Moresby",
	                         "Pacific/Bougainville", "Asia/Manila", "Asia/Karachi", "Europe/Warsaw", "America/Miquelon",
	                         "Pacific/Pitcairn", "America/Puerto_Rico", "Asia/Gaza", "Asia/Hebron", "Europe/Lisbon",
	                         "Atlantic/Madeira", "Atlantic/Azores", "Pacific/Palau", "America/Asuncion", "Asia/Qatar",
	                         "Indian/Reunion", "Europe/Bucharest", "Europe/Belgrade", "Europe/Kaliningrad", "Europe/Moscow",
	                         "Europe/Simferopol", "Europe/Volgograd", "Europe/Kirov", "Europe/Astrakhan", "Europe/Samara",
	                         "Europe/Ulyanovsk", "Asia/Yekaterinburg", "Asia/Omsk", "Asia/Novosibirsk", "Asia/Barnaul",
	                         "Asia/Tomsk", "Asia/Novokuznetsk", "Asia/Krasnoyarsk", "Asia/Irkutsk", "Asia/Chita",
	                         "Asia/Yakutsk", "Asia/Khandyga", "Asia/Vladivostok", "Asia/Ust-Nera", "Asia/Magadan",
	                         "Asia/Sakhalin", "Asia/Srednekolymsk", "Asia/Kamchatka", "Asia/Anadyr", "Africa/Kigali",
	                         "Asia/Riyadh", "Pacific/Guadalcanal", "Indian/Mahe", "Africa/Khartoum", "Europe/Stockholm",
	                         "Asia/Singapore", "Atlantic/St_Helena", "Europe/Ljubljana", "Arctic/Longyearbyen",
	                         "Europe/Bratislava", "Africa/Freetown", "Europe/San_Marino", "Africa/Dakar", "Africa/Mogadishu",
	                         "America/Paramaribo", "Africa/Juba", "Africa/Sao_Tome", "America/El_Salvador",
	                         "America/Lower_Princes", "Asia/Damascus", "Africa/Mbabane", "America/Grand_Turk",
	                         "Africa/Ndjamena", "Indian/Kerguelen", "Africa/Lome", "Asia/Bangkok", "Asia/Dushanbe",
	                         "Pacific/Fakaofo", "Asia/Dili", "Asia/Ashgabat", "Africa/Tunis", "Pacific/Tongatapu",
	                         "Europe/Istanbul", "America/Port_of_Spain", "Pacific/Funafuti", "Asia/Taipei",
	                         "Africa/Dar_es_Salaam", "Europe/Kiev", "Europe/Uzhgorod", "Europe/Zaporozhye", "Africa/Kampala",
	                         "Pacific/Johnston", "Pacific/Midway", "Pacific/Wake", "America/New_York", "America/Detroit",
	                         "America/Kentucky/Louisville", "America/Kentucky/Monticello", "America/Indiana/Indianapolis",
	                         "America/Indiana/Vincennes", "America/Indiana/Winamac", "America/Indiana/Marengo",
	                         "America/Indiana/Petersburg", "America/Indiana/Vevay", "America/Chicago",
	                         "America/Indiana/Tell_City", "America/Indiana/Knox", "America/Menominee",
	                         "America/North_Dakota/Center", "America/North_Dakota/New_Salem", "America/North_Dakota/Beulah",
	                         "America/Denver", "America/Boise", "America/Phoenix", "America/Los_Angeles", "America/Anchorage",
	                         "America/Juneau", "America/Sitka", "America/Metlakatla", "America/Yakutat", "America/Nome",
	                         "America/Adak", "Pacific/Honolulu", "America/Montevideo", "Asia/Samarkand", "Asia/Tashkent",
	                         "Europe/Vatican", "America/St_Vincent", "America/Caracas", "America/Tortola", "America/St_Thomas",
	                         "Asia/Ho_Chi_Minh", "Pacific/Efate", "Pacific/Wallis", "Pacific/Apia", "Asia/Aden", "Indian/Mayotte",
	                         "Africa/Johannesburg", "Africa/Lusaka", "Africa/Harare"};

	// minimize failure spam:
	//all_zones.erase(all_zones.begin()+3, all_zones.end());

	for (auto &zone: all_zones) {

		std::string formated = time_t_to_readable(gen_exact_date(2019,11,21,11,10,30), zone.c_str());
		std::string time_to_match = "2019-11-21T11:10:30";
		//_mark("Testing zone: " << zone << " formated: " << formated);
		EXPECT_NE(formated.find(time_to_match),std::string::npos)
		        << "Generated time ["<< formated << "] for " << zone
		        << " time zone did not match expected ["<< time_to_match <<"]";
	}
}
