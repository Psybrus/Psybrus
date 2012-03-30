//
// Protocol to receive the server details from the modal view
//

@protocol ChatServerDetailsProtocol
- (void) connectToChatServer:(NSString *)serverIP serverPort:(NSString *)serverPort;
@end
