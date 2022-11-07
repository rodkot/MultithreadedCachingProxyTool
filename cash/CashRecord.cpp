//
// Created by rodion on 06.11.22.
//

#include "CashRecord.h"
CashRecord::CashRecord(Response * res):response(res),count(0),count_active(0) {

}

void CashRecord::connect_client() {
    count_active++;
    count++;
}
void CashRecord::disconnect_client() {
    count_active--;
}