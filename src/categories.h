#ifndef CATEGORIES_H
#define CATEGORIES_H

#define TOTAL_CLASSES 10
#define TOTAL_DIVISIONS 9 
#define MENU_OPTIONS_SIZE 5


char *menu_options[MENU_OPTIONS_SIZE] = {
	"Get books by tag",
	"Get books by type",
	"Add tag to book",
	"Show books to read",
	"Set books to read"
};

/*
 * Main classes that the Dewey decimal system classifies books
 * This is the first index of a books classification
 * 000 - 900
 */
char *main_classes[10] = {
	"Computer_Science",
	"Philosophy_and_Psychology",
	"Religion",
	"Social_Sciences",
	"Language",
	"Science",
	"Technology",
	"Arts_and_Recreation",
	"Literature",
	"History_and_Geography"
};

/*
 * Heirarchical divisions that increase a books specificity
 * These are the second index in the classification system
 * 010 - 090
 */
char *division_zero[9] = {
	"Bibliography",
	"Library_and_information_sciences",
	"General_encyclopedic_works",
	"Special_topics",
	"General_serials_and_their_indexes",
	"General_organizations_and_museums",
	"News_media_journalism_publishing",
	"General_collections",
	"Manuscripts_and_rare_books"
};

/* 110 - 190 */
char *division_one[9] = {
	"Metaphysics",
	"Epistemology_causation_humankind",
	"Paranormal_phenomena",
	"Specific_philosophical_schools",
	"Psychology",
	"Logic",
	"Ethics_moral_philosophy",
	"Ancient_medieval_oriental_philosophy",
	"Modern_western_philosophy"
};

/* 210 - 290 */
char *division_two[9] = {
	"Natural_theology",
	"Bible",
	"Christian_theology",
	"Christian_moral_and_devotional_theology",
	"Christian_orders_and_local_churches",
	"Christian_social_theology",
	"Christian_church_history",
	"Christian_denominations_and_sects",
	"Other_and_comparative_religions"
};

/* 310 - 390 */
char *division_three[9] = {
	"General_statistics",
	"Political_science",
	"Economics",
	"Law",
	"Public_administration",
	"Social_problems_and_services",
	"Education",
	"Commerce_communications_transport",
	"Customs_etiquette_folklore"
};

/* 410 - 490 */
char *division_four[9] = {
	"Linguistics",
	"English_and_Anglo_Saxon_languages",
	"Germanic_languages_German",
	"Romance_languages_French",
	"Italian_Romanian_Rhaeto_Romanic",
	"Spanish_and_Portuguese_languages",
	"Italic_languages_Latin",
	"Hellenic_languages_Classical_Greek",
	"Other_languages"
};

/* 510 - 590 */
char *division_five[9] = {
	"Mathematics",
	"Astronomy_and_allied_sciences",
	"Physics",
	"Chemistry_and_allied_sciences",
	"Earth_sciences",
	"Paleontology_and_Paleozoology",
	"Life_sciences",
	"Botanical_sciences",
	"Zoological_sciences"
};

/* 610 - 690 */
char *division_six[9] = {
	"Medical_sciences_Medicine_Psychiatry",
	"Engineering",
	"Agriculture",
	"Home_economics_and_family_living",
	"Management",
	"Chemical_engineering",
	"Manufacturing",
	"Manufacture_for_specific_use",
	"Buildings"
};

/* 710 - 790 */
char *division_seven[9] = {
	"Civic_and_landscape_art",
	"Architecture",
	"Sculpture",
	"Drawings_and_decorative_arts",
	"Paintings_and_painters",
	"Graphic_arts_Printmaking_and_prints",
	"Photography",
	"Music",
	"Recreational_and_performing_arts"
};

/* 810 - 890 */
char *division_eight[9] = {
	"American_literature_in_English",
	"English_literature",
	"Literature_of_Germanic_language",
	"Literatures_of_Romance_language",
	"Italian_Romanian_Rhaeto_Romanic_Literatures",
	"Spanish_and_Portuguese_literatures",
	"Italic_literatures_Latin",
	"Hellenic_literatures_Classical_Greek",
	"Literatures_of_other_languages"
};

/* 910 - 990 */
char *division_nine[9] = {
	"Geography_and_travel",
	"Biography_genealogy_insignia",
	"History_of_the_ancient_world",
	"General_history_of_Europe",
	"General_history_of_Asia_Far_East",
	"General_history_of_Africa",
	"General_history_of_North_America",
	"General_history_of_South_America",
	"General_history_of_other_areas"
};

#endif // CATEGORIES_H

