// A
+(UIView*)flashAt:(CGRect)r in:(UIView*)parent color:(UIColor*)color;
{
	float duration = 0.5;
	UIView *flash = [[[UIView alloc] initWithFrame:r] autorelease];
	flash.backgroundColor = color;
	[parent addSubview:flash];
	[[flash invokeAfter:duration+0.1] removeFromSuperview];
		
	[UIView beginAnimations:@"SPFlash" context:NULL];
	[UIView setAnimationDuration:duration];
	flash.alpha = 0.0;
	[UIView commitAnimations];
	return flash;
}

// B
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	
	// Force the animation to happen by calling this method again after a small
	// delay - see http://blog.instapaper.com/post/53568356
	[[self nextRunloop] delayedTableViewDidSelectRowAtIndexPath: indexPath];
}

// C
[[tableView invokeAfter:0.15] selectRowAtIndexPath:indexPath animated:YES scrollPosition:UITableViewScrollPositionNone];
[[tableView invokeAfter:0.30] deselectRowAtIndexPath:indexPath animated:YES];
[[tableView invokeAfter:0.45] selectRowAtIndexPath:indexPath animated:YES scrollPosition:UITableViewScrollPositionNone];
