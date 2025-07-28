# ADSL — Advanced Data Structuring Language

A minimalist, highly-readable custom file format and parser/API for modern C++.  
**ADSL** is perfect for configuration, game/world descriptions, assets, or any scenario where structured, human-editable data is useful.

---

## Features

- **Entities:** `#` to declare objects with a list of fields  
- **Fields:** `- key=value` for data, supporting types: integers, floats, booleans, strings, and lists  
- **Groups (Tags):** `@groupName` for shared classifications and annotations  
- **Group metadata:** `@groupName[meta1,meta2]`  
- **Comments:** `//` anywhere, ignored during parsing  
- **Multigroup & multi-type fields:** Assign multiple groups to fields/entities and use lists as values  
- **Easy C++ API:** Read, write, query, and edit in code

---

## Example: `example.adsl`

```adsl
// Groups
@year2021[2021]
@employee
@vehicles

#person @employee
    - name="Alice", @hr
    - age=30
    - isActive=true
    - skills=["C++","ASM"] @tech

#person
    - name="Bob"
    - age=22
    - isActive=false @intern

#car @vehicles
    - brand="Volvo"
    - datePurchase=2021 @year2021
    - price=42000.5

#truck @vehicles
    - brand="Mercedes"
    - datePurchase=2021 @year2021
    - capacity=12.5
```

---

## Quickstart

### 1. Build

Requires **C++17**.  
No dependencies beyond the STL.

```bash
g++ -std=c++17 adsl.cpp adsl_api.cpp example.cpp -o adsl_demo
```

- For Clang, you can simply replace `g++` with `clang++`.

for (MSVC)

```bash
cl /std:c++17 adsl.cpp adsl_api.cpp example.cpp
```

or any other standard C++17 compiler.

### 2. Code Example

```cpp
#include "adsl_api.hpp"
#include <iostream>

int main() {
    adsl::API api;
    api.loadFile("example.adsl");

    // List all 'car' entities
    for (auto* car : api.entitiesByType("car")) {
        std::cout << "Car, brand = ";
        for (const auto& field : car->fields)
            if (field.name == "brand")
                std::cout << adslValueToString(field.value) << '\n';
    }

    // Add a new entity
    auto& truck = api.addEntity("truck", {"vehicles"});
    api.addField(truck, "brand", std::string("Renault"));
    api.addField(truck, "capacity", 10.0f);

    // Save result
    api.saveFile("output.adsl");
}
```

---

## API Overview

- **Parse from file:** `parseAdslFile(filename, db);`
- **Parse from string:** `parseAdslString(data, db);`
- **Serialize:** `adsl::serialize(db);`
- **High-level API:** Use `adsl::API` for everything (loading, querying, creating entities/fields/groups, saving).

### Types

- `AdslEntity` — Contains type, fields, groups
- `AdslField` — Contains name, value, groups
- `AdslGroup` — Contains name and optional values

### Supported value types:

- `int`
- `float`
- `bool` (`true`/`false`)
- `std::string` (with `"..."` syntax)
- List versions of those (e.g. `["str1","str2"]`, `[1,2,3]`, `[true,false]`)

---

## File Format Reference

- **Entity:**  
  `#<type> [@group1 @group2 ...]`
- **Field:**  
  `- <name>=<value> [@group1 @group2 ...]`
- **Group definition (optional metadata):**  
  `@<group>[meta1,meta2,...]`
- **List values:**  
  `- skills=["c++","asm"]`
- **Comments:**  
  `// This is a comment`

---

## Working on :

- Add more complex types (nesting, maps, etc) in C++
- Add file inclusion (`#include`)
- Extend the API for filtering, search, or validation

---

## License

[MIT](LICENSE)

---

Feel free to fork, improve, and share!