/**
 * createInvoiceResponse
 * 
 * @param String payment_request - Payment request to be saved from the createInvoice request
 * @param String payment_id - to be used to poll for invoice settled or not
 * 
 * 
 */
class createInvoiceResponse {
  public: 
    String payment_request;
    String payment_id;
};

/**
 * PaymentConnector
 * 
 * Skeleton for functions required to make a PaymentConnector
 * 
 */
class PaymentConnector {
  public:
    PaymentConnector(String currency_pair);
    createInvoiceResponse createInvoice(int num_satoshis, String memo);
    int checkIfPaymentIsSettled(String id);
    String createRequest(String method, String path, String data = "");
  private:
    String _currency_pair;
};
