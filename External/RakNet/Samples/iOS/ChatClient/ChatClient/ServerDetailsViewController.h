//
// Modal view used to enter the chat server IP/Port
//

#import <UIKit/UIKit.h>
#import "ChatServerDetailsProtocol.h"

@protocol ChatServerDetailsProtocol;

@interface ServerDetailsViewController : UIViewController
{
    id<ChatServerDetailsProtocol> delegate;
    UITextField* mServerIP;
    UITextField* mServerPort;
}

-(IBAction)dismissServerDetailsView;

@property (nonatomic, assign) id<ChatServerDetailsProtocol> delegate;
@property (nonatomic, retain) IBOutlet UITextField *mServerIP;
@property (nonatomic, retain) IBOutlet UITextField *mServerPort;

@end
