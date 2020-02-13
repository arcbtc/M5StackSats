class PaymentServerLND : public PaymentServer
{
  public:
   virtual boolean init();
   virtual String getServiceName();
   virtual PaymentInvoice getInvoice(int sats, String memo);
   virtual bool isInvoicePaid(String id);
};