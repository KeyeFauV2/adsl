
#include "../adsl_api.hpp"
#include <iostream>

int main()
{
    /**
     * This is a simple demo program that shows how to use the
     * ADSL API to:
     * 1) load an existing ADSL file,
     * 2) query entities by group,
     * 3) create new entities and fields programmatically,
     * 4) save the modified database to a new file,
     * 5) print the current state of the database to std::cout.
     * * Note: This is a basic example. For more complex operations, for a better documentation, see README.md.
     * * Note2: The ADSL API is designed to be easy to use and flexible, You can still use the raw ADSL parser (adsl.hpp) if you prefer to work with the underlying data structures directly.
    */
    adsl::API api;

    // 1) load an existing file
    api.loadFile("example.adsl");

    // 2) query
    auto vehicles = api.entitiesByGroup("vehicles");
    std::cout << "vehicles count = " << vehicles.size() << '\n';

    // 3) create new data programmatically
    auto& car2023 = api.addEntity("car", {"vehicles"});
    api.addField(car2023, "brand",     std::string("Tesla"));
    api.addField(car2023, "datePurchase", 2023, {"year2023"});
    api.addField(car2023, "price", 51000.0f);

    // 4) save to another file
    api.saveFile("out.adsl");

    // 5) dump to std::cout
    std::cout << "\n--- CURRENT DATABASE ---\n";
    std::cout << api.toString();
}