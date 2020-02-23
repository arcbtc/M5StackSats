class PaymentServerLND : public PaymentServer
{
  public:
   virtual boolean init();
   virtual String getServiceName();
   virtual PaymentInvoice getInvoice(int sats, String memo);
   virtual bool isInvoicePaid(String id);
  private:
   String lndServer;
   int lndRestApiPort;
   String readmacaroon;
   String invoicemacaroon;
   String tlscert;
   HTTPClient http;
   String createRequest(String method, String path, String data, String macaroon);
};