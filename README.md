# lookup
C++ Lookup Table &amp; Map (JSON Serialization)

**C++ VERSION**: C++14 or newer

The data directory contains sample 2-/3-/4-D data sets in CSV format (last column representing the table value). The 'convert' directory contains a simple CSV parser plus conversion code to generate JSON representations of each table (and combined map). 

The 'lookup' directory contains the primary implementation of the library. Source is organized as follows:
+ detail.hpp: Type & trait forward declarations, standard library aliasing, etc
+ interpolate.hpp: N-D (linear) interpolation implementation
+ json.h/json.cpp: JSON serialization adapters
+ lookup.hpp: Primary implementation for 'table' and 'table_map' types
+ utility.hpp: Algorithms implemented for 'grid' (vector-of-vectors) manipulation / access
+ traits.hpp: Type traits for accessing details of a given table / grid / array
