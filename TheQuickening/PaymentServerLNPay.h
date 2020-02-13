class PaymentServerLNPay : public PaymentServer
{
  public:
   virtual boolean init();
   virtual String getServiceName();
   virtual PaymentInvoice getInvoice(int sats, String memo);
   virtual bool isInvoicePaid(String id);
  private:
   String api_key;
   String wallet_key;
   String api_endpoint;
   String invoice_create_endpoint;
   String invoice_check_endpoint;
   HTTPClient http;
   String createRequest(String method,String path, String data);
};