//
// Created by dongmin on 18. 7. 16.
//

#ifndef TM2IN_IO_COLLADA_H
#define TM2IN_IO_COLLADA_H

#include <lib/rapidxml/rapidxml.hpp>
#include "features/Room.h"

namespace TM2IN{
    namespace io{
        /**
         * @ingroup public_api
         * @brief Imports COLLADA(dae) file and returns a vector of Room.
         */
        std::vector<Room*> importDAE(const char *filePath);

    }
}


#endif //TM2IN_IO_COLLADA_H