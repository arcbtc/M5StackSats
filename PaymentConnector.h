
class createInvoiceResponse {
  public: 
    String payment_request;
    String payment_id;
};


class PaymentConnector {
  public:
    PaymentConnector(String currency_pair);
    createInvoiceResponse createInvoice(int num_satoshis, String memo);
    int checkIfPaymentIsSettled(String id);
    String createRequest(String method, String path, String data = "");
  private:
    String _currency_pair;
};
