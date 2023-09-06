//
//  KeyImplementations.cpp
//  WMI
//
//  Created by Miroslav Yozov on 14.07.23.
//  Copyright © 2023 vit9696. All rights reserved.
//

#include "KeyImplementations.hpp"

SMC_RESULT F0Ac::readAccess() {
    uint16_t value = atomic_load_explicit(speed, memory_order_acquire);
    *reinterpret_cast<uint16_t *>(data) = VirtualSMCAPI::encodeIntFp(SmcKeyTypeFpe2, value);
    return SmcSuccess;
}
