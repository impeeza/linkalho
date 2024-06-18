#include "views/country_select_view.hpp"
#include "utils/country_list.hpp"
#include "core/shared_settings.hpp"
#include "constants.hpp"
#include <string>

using namespace std;
using namespace brls::i18n::literals;

CountrySelectView::CountrySelectView() : SelectListItem("translations/country_select_view/title"_i18n, {DEFAULT_COUNTRY}, 0)
{
    SharedSettings::instance().setCountryCode(DEFAULT_COUNTRY_CODE);
    vector<string> countryList = {};

    countryList.reserve(COUNTRIES.size());
    for (auto const& c : COUNTRIES) {
        countryList.emplace_back(c.first);
    }
    this->values.swap(countryList);
    auto it = find(this->values.begin(), this->values.end(), DEFAULT_COUNTRY);
    if (it != this->values.end()) {
        this->setSelectedValue(distance(this->values.begin(), it));
    }

    this->getValueSelectedEvent()->subscribe([this](int result) {
        SharedSettings::instance().setCountryCode(getOrDefault(COUNTRIES, this->getValue() ,DEFAULT_COUNTRY_CODE));
    });

}
