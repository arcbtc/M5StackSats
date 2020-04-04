#include <Arduino.h>

#include "PaymentInvoice.h"
#include "PaymentServer.h"

String PaymentServer::getServiceName() { return (String)"MOCK"; }

bool PaymentServer::init() {
    return true;
}

PaymentInvoice PaymentServer::getInvoice(int sats, String memo) { 
    PaymentInvoice invoice;
    invoice.paymentRequest = (String)"lnbtc....MOCK";
    invoice.id = (String)"1";
    return invoice;
}

bool PaymentServer::isInvoicePaid(String id) { return true; }