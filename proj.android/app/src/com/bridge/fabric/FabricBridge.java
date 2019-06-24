package com.bridge.fabric;

import java.math.BigDecimal;
import java.util.Currency;

import android.util.Log;

import com.crashlytics.android.answers.*;

public class FabricBridge
{
	private static final String TAG = "com.bridge.fabric";
	
	public static void logSignUpWithMethod ( final String method,
            boolean success )
	{
		Answers.getInstance().logSignUp(new SignUpEvent()
	      .putMethod(method)
	      .putSuccess(success));
	}
	public static void logLoginWithMethod ( final String method, boolean success )
	{
		Answers.getInstance().logLogin(new LoginEvent()
	      .putMethod(method)
	      .putSuccess(success));
	}
	public static void logInviteWithMethod ( final String method )
	{
		Answers.getInstance().logInvite(new InviteEvent()
	      .putMethod(method));
	}
	public static void logPurchaseWithPrice ( float price,
			final String currency,
              boolean success,
              final String itemName,
              final String itemType,
              final String itemId )
	{
		Answers.getInstance().logPurchase(new PurchaseEvent()
	      .putItemPrice(BigDecimal.valueOf(price))
	      .putCurrency(Currency.getInstance(currency))
	      .putItemName(itemName)
	      .putItemType(itemType)
	      .putItemId(itemId)
	      .putSuccess(success));
	}
	public static void logLevelStart ( final String method )
	{
		Answers.getInstance().logLevelStart(new LevelStartEvent()
	      .putLevelName(method));
	}
	public static void logLevelEnd ( final String method, int score, boolean success )
	{
		Answers.getInstance().logLevelEnd(new LevelEndEvent()
	      .putLevelName(method)
	      .putScore(score)
	      .putSuccess(success));
	}


}