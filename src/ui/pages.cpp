#include"ui.h"
extern std::shared_ptr<ui_draw>ui_create_hello();
extern std::shared_ptr<ui_draw>ui_create_network();
extern std::shared_ptr<ui_draw>ui_create_choose_image();
extern std::shared_ptr<ui_draw>ui_create_choose_disk();
extern std::shared_ptr<ui_draw>ui_create_confirm();
extern std::shared_ptr<ui_draw>ui_create_progress();
extern std::shared_ptr<ui_draw>ui_create_mass_storage();
extern std::shared_ptr<ui_draw>ui_create_mass_storage_running();
const std::vector<ui_page>pages={
	{"hello",        ui_create_hello},
	{"network",      ui_create_network},
	{"choose-image", ui_create_choose_image},
	{"choose-disk",  ui_create_choose_disk},
	{"confirm",      ui_create_confirm},
	{"progress",     ui_create_progress},
	{"mass-storage", ui_create_mass_storage},
	{"mass-running", ui_create_mass_storage_running},
};
