class PaymentServerLND : public PaymentServer
{
  public:
   virtual boolean init(String server, int restPort, String readMacaroonHex, String invoiceMacaroonHex);
   virtual String getServiceName();
   virtual PaymentInvoice getInvoice(int sats, String memo);
   virtual bool isInvoicePaid(String id);
  private:
   String lndServer;
   int lndRestApiPort;
   String readmacaroon;
   String invoicemacaroon;
   String tlscert;
   String createRequest(String method, String path, String data, String macaroon);
};
