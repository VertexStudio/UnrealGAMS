// Copyright 2018 Maksim Shestakov. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintJsonLibrary.generated.h"


UENUM(BlueprintType)
enum class EJsonType : uint8
{
	None,
	Null,
	String,
	Number,
	Boolean,
	Array,
	Object
};

USTRUCT(BlueprintType, meta = (HasNativeMake = "BlueprintJson.BlueprintJsonLibrary.JsonMake", HasNativeBreak = "BlueprintJson.BlueprintJsonLibrary.Conv_JsonObjectToString"))
struct FBlueprintJsonObject
{
	GENERATED_USTRUCT_BODY()

	TSharedPtr<class FJsonObject> Object;
};

USTRUCT(BlueprintType, meta = (HasNativeMake = "BlueprintJson.BlueprintJsonLibrary.JsonMakeString"))
struct FBlueprintJsonValue
{
	GENERATED_USTRUCT_BODY()

	TSharedPtr<class FJsonValue> Value;
};


UCLASS()
class BLUEPRINTJSON_API UBlueprintJsonLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Creates a json object
	 * 
	 * @return	The json object
	 */
	UFUNCTION(BlueprintPure, Category = "Json", meta = (NativeMakeFunc))
	static FBlueprintJsonObject JsonMake();

	/**
	 * Sets the value of the field with the specified name.
	 *
	 * @param	JsonObject	The stored json object
	 * @param	FieldName	The name of the field to set.
	 * @param	Value		The json value to set.
	 * @return	The stored json object
	 */
	UFUNCTION(BlueprintPure, Category = "Json")
	static const FBlueprintJsonObject& JsonMakeField(const FBlueprintJsonObject& JsonObject, const FString& FieldName, const FBlueprintJsonValue& Value);

	/**
	 * Sets the value of the field with the specified name.
	 * 
	 * @param	JsonObject	The stored json object
	 * @param	FieldName	The name of the field to set.
	 * @param	Value		The json value to set.
	 * @return	The stored json object
	 */
	UFUNCTION(BlueprintCallable, Category = "Json")
	static const FBlueprintJsonObject& JsonSetField(const FBlueprintJsonObject& JsonObject, const FString& FieldName, const FBlueprintJsonValue& JsonValue);

	/**
	 * Checks whether a field with the specified name exists in the json object.
	 *
	 * @param	JsonObject	The stored json object
	 * @param	FieldName	The name of the field to check.
	 * @return true if the field exists, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Json")
	static bool JsonHasField(const FBlueprintJsonObject& JsonObject, const FString& FieldName);

	/**
	 * Checks whether a field with the specified name and type exists in the object.
	 *
	 * @param	JsonObject	The stored json object
	 * @param	FieldName	The name of the field to check.
	 * @param	Type		The type of the field to check.
	 * @return true if the field exists, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Json")
	static bool JsonHasTypedField(const FBlueprintJsonObject& JsonObject, const FString& FieldName, EJsonType Type);

	/**
	 * Removes the field with the specified name.
	 * 
	 * @param	JsonObject	The stored json object
	 * @param	FieldName	The name of the field to remove.
	 * @return	The stored json object
	 */
	UFUNCTION(BlueprintCallable, Category = "Json")
	static const FBlueprintJsonObject& JsonRemoveField(const FBlueprintJsonObject& JsonObject, const FString& FieldName);

	/**
	 * Convert json object to json string
	 * 
	 * @param	JsonObject	The stored json object
	 * @param	FieldName	The name of the field to get.
	 * @return	The json value of json object
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToJsonValue (JsonObject)", CompactNodeTitle = "ToValue", BlueprintAutocast, NativeBreakFunc), Category = "Json|Convert")
	static FBlueprintJsonValue Conv_JsonObjectToJsonValue(const FBlueprintJsonObject& JsonObject, const FString& FieldName);

	/**
	 * Convert json object to json string
	 * 
	 * @param	JsonObject	The json object to convert
	 * @return	The json string
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (JsonObject)", CompactNodeTitle = "ToString", BlueprintAutocast, NativeBreakFunc), Category = "Json|Convert")
	static FString Conv_JsonObjectToString(const FBlueprintJsonObject& JsonObject);

	/**
	 * Convert json object to pretty print json string
	 *
	 * @param	JsonObject	The json object to convert
	 * @return	The json string
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToPrettyString (JsonObject)", CompactNodeTitle = "ToPrettyString", NativeBreakFunc), Category = "Json|Convert")
	static FString Conv_JsonObjectToPrettyString(const FBlueprintJsonObject& JsonObject);

	/**
	 * Convert json string to json object
	 * 
	 * @param	JsonString	The string to convert
	 * @return	The json object
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToJsonObject (String)", CompactNodeTitle = "ToJson"), Category = "Json|Convert")
	static FBlueprintJsonObject Conv_StringToJsonObject(const FString& JsonString);	

	/**
	 * Creates a json value string
	 * 
	 * @param	Value	value to set the string to
	 * @return	The blueprint json value
	 */
	UFUNCTION(BlueprintPure, Category = "Json|Make", meta=(NativeMakeFunc))
	static FBlueprintJsonValue JsonMakeString(const FString& Value);

	/**
	 * Creates a json value int
	 * 
	 * @param	Value	value to set the int to
	 * @return	The blueprint json value
	 */
	UFUNCTION(BlueprintPure, Category = "Json|Make", meta = (NativeMakeFunc))
	static FBlueprintJsonValue JsonMakeInt(int Value);

	/**
	 * Creates a json value float
	 * 
	 * @param	Value	value to set the float to
	 * @return	The blueprint json value
	 */
	UFUNCTION(BlueprintPure, Category = "Json|Make", meta = (NativeMakeFunc))
	static FBlueprintJsonValue JsonMakeFloat(float Value);

	/**
	 * Creates a json value bool

	 * @param	Value	value to set the bool to
	 * @return	The blueprint json value
	 */
	UFUNCTION(BlueprintPure, Category = "Json|Make", meta = (NativeMakeFunc))
	static FBlueprintJsonValue JsonMakeBool(bool Value);

	/**
	 * Creates a json value array
	 * 
	 * @param	Value	value to set the array to
	 * @return	The blueprint json value
	 */
	UFUNCTION(BlueprintPure, Category = "Json|Make", meta = (NativeMakeFunc))
	static FBlueprintJsonValue JsonMakeArray(const TArray<FBlueprintJsonValue>& Value);

	/**
	 * Creates a json value object
	 * 
	 * @param	Value	value to set the json object to
	 * @return	The blueprint json value
	 */
	UFUNCTION(BlueprintPure, Category = "Json|Make", meta = (NativeMakeFunc))
	static FBlueprintJsonValue JsonMakeObject(const FBlueprintJsonObject& Value);

	/**
	 * Creates a json value null
	 * 
	 * @return	The blueprint json value
	 */
	UFUNCTION(BlueprintPure, Category = "Json|Make", meta = (NativeMakeFunc))
	static FBlueprintJsonValue JsonMakeNull();

	/** Return the type of json value */
	UFUNCTION(BlueprintPure, Category = "Json|Value")
	static EJsonType JsonType(const FBlueprintJsonValue& JsonValue);

	/** Return true if the json value is null, false otherwise */
	UFUNCTION(BlueprintPure, Category = "Json|Value")
	static bool JsonIsNull(const FBlueprintJsonValue& JsonValue);

	/** Returns true if the values are equal (A == B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (JsonValue)", CompactNodeTitle = "=="), Category = "Json|Value")
	static bool EquaEqual_JsonValue(const FBlueprintJsonValue& A, const FBlueprintJsonValue& B);

	/** Returns true if the values are not equal (A != B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NotEqual (JsonValue)", CompactNodeTitle = "!="), Category = "Json|Value")
	static bool NotEqual_JsonValue(const FBlueprintJsonValue& A, const FBlueprintJsonValue& B);

	/** Converts an json value into an string */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (JsonValue)", CompactNodeTitle = "->", BlueprintAutocast, NativeBreakFunc), Category = "Json|Value")
	static FString Conv_JsonValueToString(const FBlueprintJsonValue& JsonValue);

	/** Converts an json value into an int */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToInteger (JsonValue)", CompactNodeTitle = "->", BlueprintAutocast, NativeBreakFunc), Category = "Json|Value")
	static int Conv_JsonValueToInteger(const FBlueprintJsonValue& JsonValue);

	/** Converts an json value into an float */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToFloat (JsonValue)", CompactNodeTitle = "->", BlueprintAutocast, NativeBreakFunc), Category = "Json|Value")
	static float Conv_JsonValueToFloat(const FBlueprintJsonValue& JsonValue);

	/** Converts an json value into an bool */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToBool (JsonValue)", CompactNodeTitle = "->", BlueprintAutocast, NativeBreakFunc), Category = "Json|Value")
	static bool Conv_JsonValueToBool(const FBlueprintJsonValue& JsonValue);

	/** Converts an json value into an array of json value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToArray (JsonValue)", CompactNodeTitle = "->", BlueprintAutocast, NativeBreakFunc), Category = "Json|Value")
	static TArray<FBlueprintJsonValue> Conv_JsonValueToArray(const FBlueprintJsonValue& JsonValue);

	/** Converts an json value into an json object */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToJsonObject (JsonValue)", CompactNodeTitle = "->", BlueprintAutocast, NativeBreakFunc), Category = "Json|Value")
	static FBlueprintJsonObject Conv_JsonValueToObject(const FBlueprintJsonValue& JsonValue);	
};