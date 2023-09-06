//
//  KeyImplementations.hpp
//  WMI
//
//  Created by Miroslav Yozov on 14.07.23.
//  Copyright © 2023 vit9696. All rights reserved.
//

#ifndef KeyImplementations_hpp
#define KeyImplementations_hpp

#include <IOKit/IOService.h>
#include <VirtualSMCSDK/kern_vsmcapi.hpp>

/**
 *  Key name index mapping
 */
static constexpr size_t MaxIndexCount = sizeof("0123456789ABCDEF") - 1;
static constexpr const char *KeyIndexes = "0123456789ABCDEF";

/**
 *  Key name declarations
 */
static constexpr SMC_KEY KeyFNum = SMC_MAKE_IDENTIFIER('F','N','u','m');
static constexpr SMC_KEY KeyF0Ac(size_t i) { return SMC_MAKE_IDENTIFIER('F',KeyIndexes[i],'A','c'); }

class SMIIdxKey : public VirtualSMCValue {
protected:
    size_t index;
public:
    SMIIdxKey(size_t index) : index(index) {}
};

class F0Ac : public SMIIdxKey {
    using SMIIdxKey::SMIIdxKey;
    
    _Atomic(uint16_t) *speed;
    
public:
    F0Ac(size_t index, _Atomic(uint16_t) *speed) : SMIIdxKey(index), speed(speed) {}

protected:
    SMC_RESULT readAccess() override;
};

#endif /* KeyImplementations_hpp */
