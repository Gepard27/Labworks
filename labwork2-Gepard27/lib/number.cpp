#include "number.h"
#include <cstring>

uint239_t FromInt(uint32_t value, uint32_t shift) {
    
    uint239_t val;

    for (int i = 34; i >= 0; i--) {
        val.data[i] = value % 128;
        value /= 128;
    }
    
    return ShiftFront(val, shift);
}

uint239_t ShiftNumber(uint239_t number, uint32_t c_shift) {
    bool flag = false;
    c_shift %= 7 * 35;
    for (int i = c_shift; i > 0; i--) {
        for (int j = 34; j >= 0; j--) {
            number.data[j] = (number.data[j] << 1);

            if (flag) {
                number.data[j] += 1;
            }

            if ((number.data[j] >> 7) == 1) {
                number.data[j] -= 128;
                flag = true;
            }
            else {
                flag = false;
            }
        }

        if (flag) {
            number.data[34] += 1;
            flag = false;
        }
    }
    return number;    
}

uint239_t FromString(const char* str, uint32_t shift) { 
    uint239_t number = FromInt(0,0);

    for (int i = 0; i < strlen(str) - 1; i++) {  
        number = number + FromInt(str[i] - '0', 0);
        number = number * FromInt(10,0); 
    }
    number = number + FromInt(str[strlen(str) - 1] - '0', 0);

    return ShiftFront(number, shift); 
}

uint239_t ShiftBack(uint239_t number, uint32_t shift) {
    for (int i = 0; i < 35; i++) { 
        if ((number.data[i] >> 7) == 1) { 
            number.data[i] -= 128; 
        } 
    }

    return ShiftNumber(number, (7 * 35 - (shift % (7 * 35))));  
}

uint239_t ShiftFront(uint239_t number, uint32_t shift) {
    uint239_t sum_num = ShiftNumber(number, shift);

    int i = 34;
    while (shift > 0) {
        if (shift % 2 == 1) {
            sum_num.data[i] += 128;
        }
        shift /= 2;
        i -= 1;
    }
    return sum_num; 
}


uint239_t operator+(const uint239_t& lhs, const uint239_t& rhs) {
    uint239_t num_1 = ShiftBack(lhs, GetShift(lhs));
    uint239_t num_2 = ShiftBack(rhs, GetShift(rhs));
 
    uint239_t sum_num{};
    uint8_t ost = 0;

    for (int i = 34; i >= 0; i--) {
        sum_num.data[i] += (num_1.data[i] + num_2.data[i] + ost) % 128;
        ost = (num_1.data[i] + num_2.data[i] + ost) / 128;
    }

    return ShiftFront(sum_num, GetShift(lhs) + GetShift(rhs));
}



uint239_t operator-(const uint239_t& lhs, const uint239_t& rhs) {
    bool flag = false;

    uint32_t shift;
    if (GetShift(lhs) >= GetShift(rhs)) {
        shift = GetShift(lhs) - GetShift(rhs);
    } else {
        shift = (1ll << 35) + GetShift(lhs) - GetShift(rhs); 
    }

    uint239_t num_1 = ShiftBack(lhs, GetShift(lhs));
    uint239_t num_2 = ShiftBack(rhs, GetShift(rhs));
   
    uint239_t num_sub = num_1;

    for (int i = 34; i >= 0; i--) {
        if (flag == true && num_sub.data[i] > 0) {
            num_sub.data[i] -= 1;
            flag = false;
        } else if (flag == true) {
            num_sub.data[i] += 127;
            flag = true;
        }

        if (num_sub.data[i] >= num_2.data[i]) {
            num_sub.data[i] -= num_2.data[i];
        } else {
            num_sub.data[i] += (128 - num_2.data[i]);
            flag = true;
        }
    }
   
    return ShiftFront(num_sub, shift);
}

uint239_t operator*(const uint239_t& lhs, const uint239_t& rhs) {
    uint32_t shift = GetShift(lhs) + GetShift(rhs);
    uint239_t num_1 = ShiftBack(lhs, GetShift(lhs));
    uint239_t num_2 = ShiftBack(rhs, GetShift(rhs));
    uint239_t num_mult{};
    uint32_t mult_shift = 0;


    for (int i = 34; i >= 0; i--) {
        int number = num_2.data[i]; 
        int massive_bit[8];

        for (int j = 7; j >= 0; j--){
            massive_bit[j] = number % 2;
            number /= 2;
        }

        for (int s = 7; s > 0; s--){
            if (massive_bit[s] == 1) {
                num_mult = num_mult + ShiftNumber(num_1, mult_shift);
            }
            mult_shift += 1;
        }  
    }
    
    return ShiftFront(num_mult, shift);
}

uint239_t operator/(const uint239_t& lhs, const uint239_t& rhs) {
    uint32_t shift = 0;
    if (GetShift(lhs) >= GetShift(rhs)) {
        shift = GetShift(lhs) - GetShift(rhs);
    } else {
        shift = (1ll << 35) + GetShift(lhs) - GetShift(rhs); 
    }

    uint239_t num_1 = ShiftBack(lhs, GetShift(lhs));
    uint239_t num_2 = ShiftBack(rhs, GetShift(rhs));
   
    uint239_t num_div = FromInt(0,0);

    if (num_2 == FromInt(1,0)) {
        num_div = num_1;
    }
    else {
        while (num_1 >= num_2) {
            num_1 = num_1 - num_2;
            num_div = num_div + FromInt(1, 0);
        } 
    }

    return ShiftFront(num_div, shift);
    
}


bool operator==(const uint239_t& lhs, const uint239_t& rhs) {
    uint239_t num_1 = ShiftBack(lhs, GetShift(lhs));
    uint239_t num_2 = ShiftBack(rhs, GetShift(rhs));
    for (int i = 0; i < 35; i++) {
        if (num_1.data[i] != num_2.data[i]) {
            return false;
        } 
    }
    return true;
}

bool operator!=(const uint239_t& lhs, const uint239_t& rhs) {
    return !(lhs == rhs);
}

bool operator>=(const uint239_t& lhs, const uint239_t& rhs) {
    uint239_t num_1 = ShiftBack(lhs, GetShift(lhs));
    uint239_t num_2 = ShiftBack(rhs, GetShift(rhs));

    for (int i = 0; i < 35; i++) {
        if (num_1.data[i] > num_2.data[i]) {
            return true;
        } else if (num_1.data[i] < num_2.data[i]) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& stream, const uint239_t& value) { 
    for (int i = 0; i <= 34; i++) { 
        int number = value.data[i]; 
        int massive_bit[8];
        for (int j = 7; j >= 0; j--){
            massive_bit[j] = number % 2;
            number /= 2;
        }
        for (int s = 0; s < 8; s++){
            stream << massive_bit[s]; 
        } 
    } 
    return stream; 
}

uint64_t GetShift(const uint239_t& value) {
    uint239_t number = value;
    uint32_t shift = 0;
    for (int i = 34; i >= 0; i--) {
        shift += (number.data[i] >> 7) * (1ll << (34 - i));
    }
    return shift;
}


