#include <Arduino.h>

#include "PaymentInvoice.h"
#include "PaymentServer.h"
#include "PaymentServerLND.h"

String PaymentServerLND::getServiceName() { return (String)"LND"; }

bool PaymentServerLND::init() {
    return true;
}

PaymentInvoice PaymentServerLND::getInvoice(int sats, String memo) { 
    PaymentInvoice invoice;
    invoice.paymentRequest = (String)"lnbtc....LND";
    invoice.id = (String)"1";
    return invoice;
}

bool PaymentServerLND::isInvoicePaid(String id) { return false; }