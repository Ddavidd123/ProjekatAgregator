#pragma once

class Node;
enum class OperationMode;

class Consumer {
private:
    int consumerId;
    double currentConsumption;
	
    // Veza ka parent Node-u (agregatoru)
    Node* parent;
    // Režim rada potrošača
    OperationMode mode;
    // Akumulacija za BATCH režim
    double batchAccumulated;
	
public:
    Consumer(int id, OperationMode m = (OperationMode)0);
	
    double generateConsumption();
	
    // Postavljanje parametara okruženja
    void setParent(Node* p);
    void setMode(OperationMode m);
	
    // Obrada zahteva (komande) od parent Node-a
    void handleRequest();
    // Slanje akumuliranog BATCH izveštaja parentu
    void flushBatch();
    // Reset svih lokalnih akumulacija
    void reset();
	
    // Ostatak interfejsa (zadržan radi eventualnog korišćenja)
    void sendConsumption(double value);
    int getId() const;
};